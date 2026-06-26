#include "nightscout.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
#include "config.h"

void ns_time_begin() {
  configTzTime(BSB_TZ, BSB_NTP_SERVER1, BSB_NTP_SERVER2);
}

int64_t ns_now_ms() {
  time_t now = time(nullptr);
  if (now < 1700000000) return 0;          // not synced yet (before ~2023-11)
  return (int64_t)now * 1000;
}

bool ns_fetch(Reading &out) {
  if (WiFi.status() != WL_CONNECTED) return false;

  String url = NS_URL;
  while (url.endsWith("/")) url.remove(url.length() - 1);
  url += "/api/v1/entries.json?count=";
  url += BSB_HISTORY_N;
  if (strlen(NS_TOKEN) > 0) {
    url += "&token=";
    url += NS_TOKEN;
  }

  WiFiClientSecure client;
  client.setInsecure();                    // skip cert validation (your own server)
  client.setTimeout(10);                   // seconds

  HTTPClient http;
  http.setReuse(false);
  http.setConnectTimeout(8000);
  http.setTimeout(10000);
  if (!http.begin(client, url)) {
    Serial.println("[ns] http.begin() failed");
    return false;
  }
  http.addHeader("Accept", "application/json");
  http.setUserAgent("BS-Buddy/" BSB_VERSION);

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[ns] HTTP %d (%s)\n", code, http.errorToString(code).c_str());
    http.end();
    return false;
  }

  // Read the whole body (handles chunked transfer-encoding, which trips up
  // streaming parsers on larger responses).
  String payload = http.getString();
  http.end();
  if (payload.isEmpty()) {
    Serial.println("[ns] empty response body");
    return false;
  }

  // Keep only the fields we use, so a long history stays cheap on RAM.
  JsonDocument filter;
  filter[0]["sgv"]       = true;
  filter[0]["date"]      = true;
  filter[0]["direction"] = true;

  JsonDocument doc;
  DeserializationError err =
      deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (err) {
    Serial.printf("[ns] JSON parse error: %s (len=%u)\n", err.c_str(), payload.length());
    return false;
  }

  JsonArray arr = doc.as<JsonArray>();
  if (arr.isNull() || arr.size() == 0) {
    Serial.println("[ns] empty entries array");
    return false;
  }

  JsonObject e0 = arr[0];
  out.sgv = e0["sgv"] | 0;
  if (out.sgv <= 0) {
    Serial.println("[ns] no sgv in latest entry");
    return false;
  }
  out.mmol = mgdlToMmol(out.sgv);

  const char *dir = e0["direction"] | "";
  strncpy(out.direction, dir, sizeof(out.direction) - 1);
  out.direction[sizeof(out.direction) - 1] = '\0';

  out.dateMs    = e0["date"].as<int64_t>();
  out.haveDelta = false;
  if (arr.size() >= 2) {
    int prev = arr[1]["sgv"] | 0;
    if (prev > 0) {
      out.deltaMmol = mgdlToMmol(out.sgv) - mgdlToMmol(prev);
      out.haveDelta = true;
    }
  }

  // Sparkline history: API returns newest-first, store oldest-first.
  int n = arr.size();
  if (n > BSB_HISTORY_N) n = BSB_HISTORY_N;
  for (int i = 0; i < n; i++) {
    out.history[n - 1 - i] = (int16_t)(arr[i]["sgv"] | 0);
  }
  out.historyCount = (uint8_t)n;

  out.valid = true;
  return true;
}
