#include "display.h"
#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "pins.h"
#include "config.h"

// Verified working stack for the JC3248W535 (AXS15231B over QSPI):
//   Arduino_ESP32QSPI bus -> Arduino_AXS15231B panel -> Arduino_Canvas framebuffer.
// The canvas holds the full 320x480 framebuffer in PSRAM; LVGL renders into a
// small partial buffer in internal RAM and we blit it into the canvas, then
// gfx->flush() pushes the whole canvas over QSPI.
static Arduino_DataBus *bus   = nullptr;
static Arduino_GFX     *panel = nullptr;
static Arduino_Canvas  *gfx   = nullptr;

static lv_display_t *disp     = nullptr;
static uint8_t      *draw_buf = nullptr;

static uint32_t millis_cb() { return millis(); }

static void flush_cb(lv_display_t *d, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  // Arduino_Canvas handles RGB565 byte order — do NOT byte-swap here.
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  lv_display_flush_ready(d);
}

#if LV_USE_LOG
static void lv_log_cb(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Serial.print("[LVGL] ");
  Serial.println(buf);
}
#endif

void hal_backlight_set(uint8_t pct) {
  if (pct > 100) pct = 100;
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  static bool attached = false;
  if (!attached) { ledcAttach(PIN_TFT_BL, 5000, 8); attached = true; }
  ledcWrite(PIN_TFT_BL, (uint32_t)pct * 255 / 100);
#else
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, pct > 0 ? HIGH : LOW);
#endif
}

bool hal_display_init() {
  // Native resolution is 320x480 (portrait); the canvas rotates to our chosen orientation.
  bus   = new Arduino_ESP32QSPI(PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_D0, PIN_TFT_D1, PIN_TFT_D2, PIN_TFT_D3);
  panel = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, BOARD_PANEL_W, BOARD_PANEL_H);
  gfx   = new Arduino_Canvas(BOARD_PANEL_W, BOARD_PANEL_H, panel, 0, 0, BOARD_ROTATION);

  if (!gfx->begin(40000000UL)) {
    Serial.println("[display] gfx->begin() FAILED — check PSRAM (must be OPI) and QSPI wiring");
    return false;
  }
  gfx->fillScreen(0x0000); // RGB565 black
  hal_backlight_set(BSB_BRIGHTNESS_NORMAL);

  lv_init();
  lv_tick_set_cb(millis_cb);
#if LV_USE_LOG
  lv_log_register_print_cb(lv_log_cb);
#endif

  uint32_t w = gfx->width();   // 480 after rotation
  uint32_t h = gfx->height();  // 320

  // Partial render buffer (~1/10 screen) in internal RAM. RGB565 = 2 bytes/px.
  uint32_t bufBytes = (w * h / 10) * 2;
  draw_buf = (uint8_t *)heap_caps_malloc(bufBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!draw_buf) {
    Serial.println("[display] draw buffer alloc failed");
    return false;
  }

  disp = lv_display_create(w, h);
  lv_display_set_flush_cb(disp, flush_cb);
  lv_display_set_buffers(disp, draw_buf, NULL, bufBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

  Serial.printf("[display] init OK (%lux%lu, rot %d)\n", (unsigned long)w, (unsigned long)h, BOARD_ROTATION);
  return true;
}

void hal_display_loop() {
  lv_timer_handler();
  if (gfx) gfx->flush();
}
