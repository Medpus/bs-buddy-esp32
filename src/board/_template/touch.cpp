// ============================================================================
//  BOARD TEMPLATE — touch driver. Implements the HAL touch contract
//  (src/hal/touch.h). If your board has NO touch, return false from
//  hal_touch_init() and leave hal_touch_read() as a no-op.
//
//  Common controllers: GT911, CST816, FT6x36 (capacitive I2C), XPT2046
//  (resistive SPI, needs runtime calibration). Reference: AXS15231B in
//  src/board/jc3248w535/touch.cpp.
// ============================================================================
#include "touch.h"   // hal/ (via -I)
#include <Arduino.h>
#include "pins.h"    // local pins + geometry

bool hal_touch_init() {
  // TODO: init your controller (Wire.begin(...) / SPI). Return false if absent.
  return false;
}

void hal_touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
  LV_UNUSED(indev);
  // TODO: read raw point, clamp/calibrate, map to logical coords per
  //       BOARD_ROTATION (see reference board), then:
  //   data->point.x = ...; data->point.y = ...;
  //   data->state = LV_INDEV_STATE_PRESSED;   // or RELEASED when no touch
  data->state = LV_INDEV_STATE_RELEASED;
}
