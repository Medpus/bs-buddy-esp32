# Adding a board

BS-Buddy keeps the app logic (Nightscout, WiFi, the UI) separate from the
board-specific bits (the panel and touch driver). So getting it onto a different
ESP32 screen means writing one small folder, and you don't touch the rest.

> Heads up: only call a board "tested" if you've actually run it. These modules
> vary between revisions (sometimes even the pins), so "it compiled" isn't the
> same as "it works".

## The pieces

```
src/core/   the app: Nightscout, WiFi, time, glucose logic → builds a ViewModel
src/view/   the LVGL screen: reads the ViewModel and draws
src/hal/    the little contract each board fills in (display.h, touch.h, board.h)
src/board/  one folder per board (the only place you add code)
```

## How

1. **Copy the template:**
   ```bash
   cp -r src/board/_template src/board/<your-board>
   ```
   Fill in the four files against `src/hal/*`:
   - `pins.h`: your pins plus screen size and rotation
   - `display.cpp`: bring up the panel (any bus/controller)
   - `touch.cpp`: read touch into coordinates (or just `return false` if there's none)
   - `board.cpp`: name and resolution in `hal_board_info()`

   `src/board/jc3248w535/` is a complete working example to copy from.

2. **Add a board JSON** at `boards/<your-board>.json` (MCU, flash, PSRAM…). Copy
   `boards/jc3248w535.json` and tweak; PlatformIO wants board JSONs in `boards/`.

3. **Add a build env** in `platformio.ini`:
   ```ini
   [env:<your-board>]
   platform         = ...
   board            = <your-board>
   build_src_filter = ${env.build_src_filter} +<board/<your-board>/>
   build_flags      = ${env.build_flags} -D BSB_BOARD_<YOUR_BOARD>
   ```
   The shared `[env]` already sets up the include paths and pulls in
   `core` / `view` / `hal`; your `+<board/...>` line just adds your board.

4. **Build and flash:** `pio run -e <your-board> -t upload`, then check the value,
   colours, trend arrow, and touch all behave.

That's it. Want to share it? Open a PR and add a row to the board table in the
README, noting whether you've run it on real hardware. No ceremony needed.

## Things that might trip you up

- **Fonts** are sized for the 480×320 reference screen. A very different resolution
  may want its own fonts (`scripts/gen_fonts.sh`) and some layout tweaks in
  `src/view/ui.cpp`.
- **Colours look wrong?** Some SPI panels need `lv_draw_sw_rgb565_swap()` in the
  flush callback; the reference QSPI board must *not* swap. Easy to spot, easy to flip.
- **Blank screen?** Usually the PSRAM settings in the board JSON (`psram_type`,
  `memory_type`), especially for big RGB panels.
