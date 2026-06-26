#include "touch.h"
#include <Arduino.h>
#include <Wire.h>
#include "pins.h"   // pins + display geometry (BOARD_SCREEN_W/H, BOARD_ROTATION)

// AXS15231B "read first touch point" command.
static const uint8_t READ_CMD[8] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08};

bool hal_touch_init() {
  Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL);
  Wire.setClock(400000);
  return true;   // AXS15231B touch is always present on this module
}

// Read a raw touch point in the native (portrait) coordinate frame.
static bool read_raw(uint16_t *rx, uint16_t *ry) {
  Wire.beginTransmission(TOUCH_I2C_ADDR);
  Wire.write(READ_CMD, sizeof(READ_CMD));
  if (Wire.endTransmission() != 0) return false;

  uint8_t b[8] = {0};
  Wire.requestFrom((int)TOUCH_I2C_ADDR, (int)sizeof(b));
  uint8_t i = 0;
  while (i < sizeof(b) && Wire.available()) b[i++] = Wire.read();
  if (i < 6) return false;

  uint8_t fingers = b[1];
  if (fingers == 0 || fingers == 0xFF) return false;  // no touch

  *rx = ((uint16_t)(b[2] & 0x0F) << 8) | b[3];
  *ry = ((uint16_t)(b[4] & 0x0F) << 8) | b[5];
  return true;
}

void hal_touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
  LV_UNUSED(indev);
  uint16_t rx, ry;
  if (!read_raw(&rx, &ry)) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  // Clamp to the calibrated raw range, then map to portrait ideal coordinates.
  if (rx < TOUCH_RAW_X_MIN) rx = TOUCH_RAW_X_MIN;
  if (rx > TOUCH_RAW_X_MAX) rx = TOUCH_RAW_X_MAX;
  if (ry < TOUCH_RAW_Y_MIN) ry = TOUCH_RAW_Y_MIN;
  if (ry > TOUCH_RAW_Y_MAX) ry = TOUCH_RAW_Y_MAX;

  uint16_t px = map(rx, TOUCH_RAW_X_MIN, TOUCH_RAW_X_MAX, 0, BOARD_SCREEN_H - 1); // 0..319
  uint16_t py = map(ry, TOUCH_RAW_Y_MIN, TOUCH_RAW_Y_MAX, 0, BOARD_SCREEN_W - 1); // 0..479

  // Rotate portrait -> landscape (rotation 1).
#if BOARD_ROTATION == 1
  data->point.x = py;
  data->point.y = (BOARD_SCREEN_H - 1) - px;
#elif BOARD_ROTATION == 3
  data->point.x = (BOARD_SCREEN_W - 1) - py;
  data->point.y = px;
#elif BOARD_ROTATION == 2
  data->point.x = (BOARD_SCREEN_H - 1) - px;
  data->point.y = (BOARD_SCREEN_W - 1) - py;
#else // 0
  data->point.x = px;
  data->point.y = py;
#endif
  data->state = LV_INDEV_STATE_PRESSED;
}
