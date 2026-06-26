#pragma once
// ============================================================================
//  HAL board contract. Every board reports its identity and logical screen
//  geometry through hal_board_info(). The view uses this to lay itself out; it
//  never reads board pins/headers directly.
//
//  NB: this is the HAL-wide contract (one per firmware). A board's own pins and
//  geometry constants live in src/board/<board>/pins.h, included only by that
//  board's .cpp files — keep them out of the shared include path to avoid an
//  ambiguous "board.h".
// ============================================================================
#include <stdint.h>

struct BoardInfo {
  const char* name;     // human-readable, e.g. "Guition JC3248W535"
  uint16_t    width;    // logical width  in px (AFTER rotation)
  uint16_t    height;   // logical height in px
  bool        hasTouch; // false -> main() skips registering an LVGL input device
};

// Implemented by the active board (src/board/<board>/board.cpp).
const BoardInfo& hal_board_info();
