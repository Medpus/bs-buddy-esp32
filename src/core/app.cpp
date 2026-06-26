#include "app.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <string.h>

#include "config.h"
#include "glucose.h"
#include "viewmodel.h"
#include "net.h"
#include "nightscout.h"
#include "ui.h"        // view/ (via -I)
#include "display.h"   // hal/  (via -I) — hal_display_loop, hal_backlight_set

static Reading   g_reading;
static int       g_failStreak = 0;     // consecutive failed polls (debounced warning)
static uint32_t  g_lastPoll   = 0;
static uint32_t  g_lastUiTick = 0;
static ViewModel g_vm;

// Brightness: tap to cycle NORMAL -> LOW -> OFF -> NORMAL.
static const uint8_t BRIGHTNESS[] = {
  BSB_BRIGHTNESS_NORMAL, BSB_BRIGHTNESS_LOW, BSB_BRIGHTNESS_OFF
};
static uint8_t g_brightIdx = 0;

void app_on_tap() {
  g_brightIdx = (g_brightIdx + 1) % (sizeof(BRIGHTNESS) / sizeof(BRIGHTNESS[0]));
  hal_backlight_set(BRIGHTNESS[g_brightIdx]);
}

static void show_status(const char *msg) {
  g_vm.screen = ViewModel::Screen::Status;
  strncpy(g_vm.statusMsg, msg, sizeof(g_vm.statusMsg) - 1);
  g_vm.statusMsg[sizeof(g_vm.statusMsg) - 1] = '\0';
  ui_render(g_vm);
  hal_display_loop();   // push the frame before any blocking call (e.g. wifi_begin)
}

void app_init() {
  show_status("Kobler til WiFi...");

  if (wifi_begin()) {
    Serial.printf("[wifi] connected, IP=%s RSSI=%d\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    show_status("Synkroniserer tid...");
    ns_time_begin();
    show_status("Henter glukose...");
  } else {
    Serial.println("[wifi] connection FAILED");
    show_status("WiFi feilet");
  }

  g_lastPoll = millis() - BSB_POLL_INTERVAL_MS;   // force an immediate first poll
}

void app_tick(uint32_t now) {
  // --- Poll Nightscout ---
  uint32_t interval = g_reading.valid ? BSB_POLL_INTERVAL_MS : BSB_RETRY_INTERVAL_MS;
  if (now - g_lastPoll >= interval) {
    g_lastPoll = now;
    wifi_ensure();
    if (wifi_connected()) {
      Reading r;
      if (ns_fetch(r)) {
        g_reading = r;
        g_failStreak = 0;
        Serial.printf("[ns] sgv=%d mmol=%.1f dir=%s delta=%+.1f hist=%d\n",
                      r.sgv, r.mmol, r.direction, r.deltaMmol, r.historyCount);
        g_lastUiTick = now - 1000;   // force an immediate UI refresh this tick
      } else {
        g_failStreak++;
      }
    } else {
      g_failStreak++;
    }

    // Before the first reading, keep the user informed on the status screen.
    if (!g_reading.valid) {
      show_status(wifi_connected() ? "Venter på data" : "WiFi frakoblet");
    }
  }

  // --- Refresh UI ~1 Hz (age, stale, clock) ---
  if (now - g_lastUiTick >= 1000) {
    g_lastUiTick = now;

    time_t t = time(nullptr);
    bool timeValid = t > 1700000000;          // NTP synced?
    int hh = 0, mm = 0;
    if (timeValid) {
      struct tm lt;
      localtime_r(&t, &lt);
      hh = lt.tm_hour;
      mm = lt.tm_min;
    }

    if (g_reading.valid) {
      bool ageValid = false;
      int  ageMin   = 0;
      bool stale    = false;
      if (timeValid && g_reading.dateMs > 0) {
        int64_t ageMs = (int64_t)t * 1000 - g_reading.dateMs;
        if (ageMs < 0) ageMs = 0;             // guard clock skew
        ageMin = (int)(ageMs / 60000);
        stale  = ageMs > (int64_t)BSB_STALE_AFTER_MS;
        ageValid = true;
      }
      bool connProblem = !wifi_connected() || g_failStreak >= 2;

      g_vm.screen       = ViewModel::Screen::Reading;
      g_vm.mmol         = g_reading.mmol;
      g_vm.zone         = classify(g_reading.mmol);
      g_vm.trendGlyph   = trendArrow(g_reading.direction);   // view applies stale -> "?"
      g_vm.haveDelta    = g_reading.haveDelta;
      g_vm.deltaMmol    = g_reading.deltaMmol;
      g_vm.ageValid     = ageValid;
      g_vm.ageMin       = ageMin;
      g_vm.stale        = stale;
      g_vm.connProblem  = connProblem;
      g_vm.timeValid    = timeValid;
      g_vm.hh           = hh;
      g_vm.mm           = mm;
      memcpy(g_vm.history, g_reading.history, sizeof(g_vm.history));
      g_vm.historyCount = g_reading.historyCount;
      ui_render(g_vm);
    }
  }
}
