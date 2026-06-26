# Adding a board

BS-Buddy splits into **device-independent** code and a thin **per-board** layer.
Adding a new ESP32 display module means writing one small board directory and one
build env — you never touch the backend or the UI.

> ⚠️ Only send a PR for a board you have **physically tested**. Per-revision pin
> differences are common (even the reference board has disputed touch pins). A
> board that merely compiles is not "supported".

## Architecture in one minute

```
src/core/   backend — Nightscout, WiFi, time, glucose logic. Produces a ViewModel.
src/view/   LVGL UI — reads a ViewModel, draws. No network/panel knowledge.
src/hal/    the contract every board implements (display.h, touch.h, board.h).
src/board/  one directory per board — the ONLY place a new board adds code.
```

Data flows `core → ViewModel → view`, and the view talks to your panel only
through LVGL + the HAL. Your board provides four things; nothing else changes.

## Steps

### 1. Create the board directory
```bash
cp -r src/board/_template src/board/<your-board>
```
Implement the four files against `src/hal/*`:

| File | Implements | What to do |
|---|---|---|
| `pins.h` | — | Pins + `BOARD_PANEL_W/H`, `BOARD_ROTATION`, `BOARD_SCREEN_W/H` |
| `display.cpp` | `hal/display.h` | Bring up the panel, register the LVGL flush cb |
| `touch.cpp` | `hal/touch.h` | Read touch → logical coords. No touch? `return false` |
| `board.cpp` | `hal/board.h` | `hal_board_info()` — name, logical W/H, hasTouch |

`src/board/jc3248w535/` is the worked reference (QSPI panel + I²C touch).

### 2. Add the PlatformIO board JSON
Drop `boards/<your-board>.json` describing the **hardware** (MCU, flash size,
PSRAM type, partitions). Copy `boards/jc3248w535.json` and adjust. This is
separate from the firmware code on purpose — PlatformIO requires board JSONs in
`boards/`.

### 3. Add a build env
In `platformio.ini`, add an env that re-includes only your board's source and
defines its board macro:
```ini
[env:<your-board>]
platform         = ...            ; your core/platform
board            = <your-board>   ; matches boards/<your-board>.json
build_src_filter = ${env.build_src_filter} +<board/<your-board>/>
build_flags      = ${env.build_flags} -D BSB_BOARD_<YOUR_BOARD>
```
The shared `[env]` already adds `src/{core,view,hal}` to the include path and
excludes all of `src/board/` — your `+<board/...>` line opts your board back in.
Boards never clash because only one board dir compiles per env.

### 4. Build, flash, verify
```bash
pio run -e <your-board> -t upload
pio device monitor
```
Check: boot status messages → glucose value with correct color/arrow → tap
cycles brightness → touch coordinates line up with what you press.

### 5. Mark its tier and send the PR
Add a `README.md` in your board dir noting **Verified on hardware** (or
*Experimental*), and add a row to the board table in the top-level `README.md`.

## Contributing your board (PR checklist)

New boards are welcome — the whole point of the layered structure is that they're
a self-contained, low-risk addition. A board PR should:

- [ ] Add **only** `src/board/<name>/` + `boards/<name>.json` + one `[env:<name>]`
      — no changes to `src/core/` or `src/view/` (if you needed to touch those,
      say why in the PR; it may belong in the shared layer).
- [ ] Build cleanly: `pio run -e <name>`.
- [ ] Be **tested on the physical board** — note this in the board's `README.md`
      and add a row to the table in the top-level `README.md`.
- [ ] Tier it honestly: **Verified** (you ran it) vs **Experimental** (compiles,
      not yet confirmed on hardware). Don't mark a board verified you haven't run.
- [ ] Include a photo or short note of it working, if you can — helps reviewers.

Reviewers can't test hardware they don't own, so the tier label and your testing
notes are what the "supported" claim rests on. Be accurate.

## Notes & gotchas

- **Fonts are resolution-specific.** The bundled fonts (`src/view/fonts/`) are
  sized for 480×320. A very different resolution may need its own font set —
  regenerate with `scripts/gen_fonts.sh`. The layout in `src/view/ui.cpp` is
  currently fixed for the reference board; a small screen may need layout tweaks.
- **Color byte order** varies. Some SPI panels need `lv_draw_sw_rgb565_swap()` in
  the flush cb; the reference QSPI+Canvas board must NOT swap. Verify visually.
- **PSRAM** — large framebuffers (RGB panels especially) need PSRAM enabled in
  the board JSON (`psram_type`, `memory_type`). A wrong setting → blank screen.
