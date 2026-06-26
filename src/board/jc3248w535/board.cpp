#include "board.h"   // HAL contract (BoardInfo) — from src/hal via -I
#include "pins.h"    // this board's geometry (local)

const BoardInfo& hal_board_info() {
  static const BoardInfo info = {
    "Guition JC3248W535",
    BOARD_SCREEN_W,   // 480 (logical, after rotation)
    BOARD_SCREEN_H,   // 320
    true,             // has AXS15231B capacitive touch
  };
  return info;
}
