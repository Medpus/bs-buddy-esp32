#pragma once
// ============================================================================
//  App logic — device-independent. Owns the poll loop and builds the ViewModel
//  the view renders. Knows nothing about the panel beyond the HAL display hooks.
// ============================================================================
#include <stdint.h>

// Boot sequence: WiFi connect, time sync, arm first poll. Call once after the
// display + UI are built (so status messages can be shown).
void app_init();

// Poll Nightscout when due, refresh the ViewModel ~1 Hz and render. Call every
// loop iteration with millis().
void app_tick(uint32_t now);

// Cycle backlight brightness NORMAL -> LOW -> OFF. Call from the screen tap event.
void app_on_tap();
