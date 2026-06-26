# Board: Guition JC3248W535

**Reference / verified board.** 3.5″ 320×480 all-in-one ESP32-S3 module.

| | |
|---|---|
| MCU | ESP32-S3-WROOM-1 **N16R8** (16 MB flash, 8 MB **octal** PSRAM) |
| Display | **AXS15231B** over QSPI, 320×480 IPS → rotated to 480×320 landscape |
| Touch | AXS15231B capacitive, I²C `0x3B` (polled) |
| Build | `boards/jc3248w535.json` (OPI PSRAM), env `jc3248w535` |
| Status | ✅ Verified on hardware |

Full pin map, gotchas (octal PSRAM, broken HW rotation, byte order) and sources:
[`docs/HARDWARE.md`](../../../docs/HARDWARE.md).

Files: `display.cpp` (QSPI panel + Arduino_Canvas + LVGL), `touch.cpp` (polled
I²C), `board.cpp` (`hal_board_info`), `pins.h` (pins + geometry).
