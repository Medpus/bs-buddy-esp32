#pragma once
// ============================================================================
//  HAL display contract. Implemented per board in src/board/<board>/display.cpp.
//  The board brings up its panel (any bus/controller) and registers an LVGL
//  display + flush callback inside hal_display_init().
// ============================================================================
#include <stdint.h>

// Bring up the panel + LVGL display driver. Returns false on failure
// (e.g. PSRAM/bus init) — main() then continues for serial diagnostics.
bool hal_display_init();

// Pump LVGL and push the framebuffer to the panel. Call every loop iteration.
void hal_display_loop();

// Set backlight brightness 0-100 % (PWM where supported, else on/off).
void hal_backlight_set(uint8_t pct);
