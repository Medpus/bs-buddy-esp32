#include "board.h"
#include <Arduino.h>
#include <Wire.h>
#include "pins.h"

static constexpr uint8_t PMU_ADDR = 0x6A;
static constexpr uint8_t PMU_TIMER_REG = 0x07;
static constexpr uint8_t PMU_DISABLE_STATUS_LED = 1U << 6;

static bool disable_status_led() {
  Wire.beginTransmission(PMU_ADDR);
  Wire.write(PMU_TIMER_REG);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(PMU_ADDR, (uint8_t)1) != 1) {
    return false;
  }

  uint8_t value = Wire.read();
  Wire.beginTransmission(PMU_ADDR);
  Wire.write(PMU_TIMER_REG);
  Wire.write(value | PMU_DISABLE_STATUS_LED);
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
