#include "net.h"
#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include "config.h"

bool wifi_connected() { return WiFi.status() == WL_CONNECTED; }

bool wifi_begin() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < BSB_WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  return wifi_connected();
}

void wifi_ensure() {
  if (wifi_connected()) return;
  Serial.println("[wifi] link down — reconnecting");
  WiFi.disconnect();
  WiFi.reconnect();
}
