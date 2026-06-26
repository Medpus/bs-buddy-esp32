// BOARD TEMPLATE — reports board identity + logical geometry to the view.
#include "board.h"   // HAL contract (from src/hal via -I)
#include "pins.h"    // this board's geometry (local)

const BoardInfo& hal_board_info() {
  static const BoardInfo info = {
    "My Board Name",   // TODO
    BOARD_SCREEN_W,
    BOARD_SCREEN_H,
    true,              // TODO: false if the board has no touch
  };
  return info;
}
