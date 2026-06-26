# Board template

Copy this directory to `src/board/<your-board>/` and implement the four files
against the HAL contract in `src/hal/`. See [`docs/ADDING_A_BOARD.md`](../../../docs/ADDING_A_BOARD.md)
for the full walkthrough.

| File | Implements | Notes |
|---|---|---|
| `pins.h` | — | Pins + display geometry for this board (local include only) |
| `display.cpp` | `hal/display.h` | Panel bring-up + LVGL flush |
| `touch.cpp` | `hal/touch.h` | Touch read, or `return false` if no touch |
| `board.cpp` | `hal/board.h` | `hal_board_info()` — name + logical W/H + hasTouch |

This `_template` directory is **excluded from every build** (`build_src_filter`
in `platformio.ini` strips `src/board/`, and no env re-adds `_template`), so the
stubs here never have to compile.
