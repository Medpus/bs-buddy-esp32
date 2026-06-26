#pragma once
// ============================================================================
//  HAL touch contract. Implemented per board in src/board/<board>/touch.cpp.
//  A board without a touch panel returns false from hal_touch_init(); main()
//  then skips registering an LVGL input device.
// ============================================================================
#include <lvgl.h>

// Initialise the touch controller. Returns false if the board has no touch
// (or init failed) -> no input device is registered.
bool hal_touch_init();

// LVGL pointer input-device read callback (already in logical screen coords).
void hal_touch_read(lv_indev_t *indev, lv_indev_data_t *data);
