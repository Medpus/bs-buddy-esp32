#include "board.h"
#include <Arduino.h>
#include <Wire.h>
#include "pins.h"

static constexpr uint8_t PMU_ADDR = 0x6A;
static constexpr uint8_t PMU_TIMER_REG = 0x07;
static constexpr uint8_t PMU_DISABLE_STATUS_LED = 1U << 6;
static constexpr uint8_t PMU_WATCHDOG_MASK = 3U << 4;

static bool disable_status_led() {
  Wire.beginTransmission(PMU_ADDR);
  Wire.write(PMU_TIMER_REG);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(PMU_ADDR, (uint8_t)1) != 1) {
    return false;
  }

  // An I2C write enters host mode. Disable its watchdog too, otherwise the PMU
  // restores register defaults after 40 seconds and the status LED starts blinking again.
  uint8_t value = Wire.read();
  value = (value & ~PMU_WATCHDOG_MASK) | PMU_DISABLE_STATUS_LED;
  Wire.beginTransmission(PMU_ADDR);
  Wire.write(PMU_TIMER_REG);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

void hal_board_init() {
  Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL);

  if (disable_status_led()) {
    Serial.println("[pmu] status LED disabled");
  } else {
    Serial.println("[pmu] SY6970 not detected; status LED remains enabled");
  }
}
