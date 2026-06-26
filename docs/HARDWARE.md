# Hardware — Guition JC3248W535 (reference board)

A 3.5″ all-in-one ESP32-S3 module. No external wiring is needed; this page
documents the on-board pin map and the gotchas that matter for firmware.

> This is the **reference board**. Its firmware pin/geometry defines live in
> [`src/board/jc3248w535/pins.h`](../src/board/jc3248w535/pins.h). Other boards
> have their own `src/board/<board>/` directory and README — to add one, see
> [`ADDING_A_BOARD.md`](ADDING_A_BOARD.md).

## Module

- **MCU:** ESP32-S3-WROOM-1 **N16R8** — 16 MB QIO flash + **8 MB OCTAL (OPI) PSRAM**
- **Display controller:** **AXS15231B** (also integrates the capacitive touch)
- **Display:** 320×480 IPS, RGB565, driven over **QSPI @ 40 MHz**
- **Touch:** AXS15231B capacitive, I²C @ 400 kHz, address `0x3B`
- **USB:** native ESP32-S3 USB (no UART bridge), VID:PID `303A:1001`

Sold as JC3248W535 / JC3248W535C / JC3248W535EN — electrically equivalent for
the display bus; touch INT/RST wiring can differ between revisions (see below).

## Pin map

### Display (AXS15231B, QSPI)

| Signal | GPIO |
|---|---|
| CS    | 45 |
| SCLK  | 47 |
| D0    | 21 |
| D1    | 48 |
| D2    | 40 |
| D3    | 39 |
| RST   | — (none; software reset over QSPI) |
| TE    | 38 (optional anti-tearing) |
| Backlight | 1 (active **HIGH**, PWM-capable) |

`Arduino_ESP32QSPI(CS, SCLK, D0, D1, D2, D3)` → `(45, 47, 21, 48, 40, 39)`.

### Touch (AXS15231B, I²C)

| Signal | GPIO |
|---|---|
| SDA | 4 |
| SCL | 8  *(shared with the panel's unused DC line)* |
| INT | **disputed** — 3, 11, or unused depending on board revision |
| RST | **disputed** — 12 or unused |
| Addr | `0x3B` |

This firmware **polls** touch over I²C, so the INT/RST disagreement doesn't
matter. If you want interrupt-driven touch, confirm the INT pin on *your* unit.

## Gotchas that bite people

1. **PSRAM is OCTAL.** Build with `memory_type = qio_opi` (quad flash + octal
   PSRAM) and `-DBOARD_HAS_PSRAM`. A plain `esp32-s3-devkitc-1` defaults to quad
   PSRAM → the ~300 KB display canvas fails to allocate → blank screen. This repo
   ships `boards/jc3248w535.json` with the correct memory layout.
2. **Hardware rotation is broken** on this panel (`swap_xy`/MADCTL → black screen).
   Rotate in **software** via the `Arduino_Canvas` rotation argument (we use `1`
   for landscape 480×320).
3. **Color byte order:** with `Arduino_Canvas` + `draw16bitRGBBitmap`, do **not**
   also call `lv_draw_sw_rgb565_swap` — that double-swaps and gives wrong colors.
4. **Don't repurpose GPIO8.** It's the touch I²C SCL; in QSPI mode the panel DC is
   unused (`dc = -1`) so the pin only serves touch.
5. **Native USB only.** No CH340/CP2102. The port is `303A:1001`; flashing uses
   the native USB-Serial/JTAG. See [FLASHING.md](FLASHING.md).

## Sources

Pin map and behavior cross-referenced from independent primary/community sources:

- ESP-IDF BSP: <https://github.com/NorthernMan54/JC3248W535EN>
- Arduino driver: <https://github.com/me-processware/JC3248W535-Driver>
- LVGL test repo: <https://github.com/byte-me404/JC3248W535_lvgl_test>
- Espressif AXS15231B component: <https://components.espressif.com/components/espressif/esp_lcd_axs15231b>
- AXS15231B datasheet (hosted by Espressif):
  <https://dl.espressif.com/AE/esp_iot_solution/AXS15231B_Datasheet_V0.5_20230306.pdf>
- ESPHome config thread: <https://community.home-assistant.io/t/jc3248w535-guition-3-5-config/791363>
