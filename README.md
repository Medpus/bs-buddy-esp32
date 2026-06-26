# BS-Buddy

An always-on **Nightscout blood sugar display** for cheap, off-the-shelf ESP32
display modules. The reference (and currently only verified) board is the
**Guition JC3248W535** — a 3.5″ 320×480 capacitive-touch ESP32-S3 module — but the
firmware is structured so other modules can be added without touching the app
logic. See [Supported boards](#supported-boards) and
[`docs/ADDING_A_BOARD.md`](docs/ADDING_A_BOARD.md).

It shows your current glucose as a large number that turns red/yellow/green at a
glance, with a trend arrow, the change since the last reading, how long ago the
reading is, and a ~2-hour sparkline of recent values.

> ⚠️ **Not a medical device.** For convenience only — never make treatment
> decisions from this display. Always confirm with your CGM/receiver or a meter.

```
 +----------------------------------------------------+
 | ingen nett          Lav                   14:32   |  ← (warning) (alert) (clock)
 |                                                    |
 |  7.8  ↗                                            |  ← big color-coded value + arrow
 |  mmol/L                          +0.3 · 3 min siden|  ← units        delta · age
 |       __                                           |
 |   ___/  \______/\___                               |  ← ~2 h sparkline
 +----------------------------------------------------+
```

The top row is conditional: the **alert word** (`Lav`, `Høy`, …) appears only when
you're out of range, the **clock** once NTP time has synced, and the `ingen nett`
warning only after WiFi/Nightscout has been unreachable for a while.

## Features

- **Large color-coded value** in mmol/L — the number turns red/yellow/green/orange
  by zone (urgent-low / low / in-range / high / urgent-high)
- **Trend arrow** driven by the Nightscout direction (`Flat`, `SingleUp`, `FortyFiveDown`, …)
- **Delta** since the previous reading (e.g. `+0.3`)
- **“N min siden”** age indicator with an explicit **stale state** (greys everything out after 13 min)
- **~2-hour sparkline** of recent readings along the bottom
- **Out-of-range alert word** (`Lav` / `Høy` / …) at the top — stays hidden while you're in range
- **Clock** (top-right) once NTP time is synced
- **Connectivity warning** (`ingen nett`) when WiFi/Nightscout has been unreachable for a while
- **Tap the screen** to cycle backlight brightness: normal → dim → off
- Crisp custom fonts, robust WiFi reconnect, NTP-based age, polled touch (no flaky INT pin)

## Hardware (reference board)

| | |
|---|---|
| Board | **Guition JC3248W535** (a.k.a. JC3248W535C / JC3248W535EN) |
| MCU | ESP32-S3-WROOM-1 **N16R8** (16 MB flash, 8 MB **octal** PSRAM) |
| Display | **AXS15231B** over QSPI, 320×480 IPS |
| Touch | AXS15231B capacitive, I²C `0x3B` |
| Power | USB-C (data cable — see flashing notes) |

No wiring required — it’s an all-in-one module. Full pin map and sources in
[`docs/HARDWARE.md`](docs/HARDWARE.md). Other boards document their own hardware in
`src/board/<board>/README.md`; to add one see [`docs/ADDING_A_BOARD.md`](docs/ADDING_A_BOARD.md).

## Quick start

1. **Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/)**
   (or the VS Code extension).

2. **Clone & configure credentials:**
   ```bash
   git clone <this-repo> bs-buddy-esp32-s3
   cd bs-buddy-esp32-s3
   cp include/secrets.example.h include/secrets.h
   $EDITOR include/secrets.h        # WiFi + Nightscout URL + token
   ```
   `secrets.h` is git-ignored. To read a locked-down Nightscout site you need a
   **read-only token** — see [`docs/NIGHTSCOUT.md`](docs/NIGHTSCOUT.md).

3. **Build & flash:**
   ```bash
   pio run -t upload      # build + flash over USB (default board: jc3248w535)
   pio device monitor     # watch the serial log @ 115200
   ```
   `pio run` targets the default board; pass `-e <board>` to pick another (each
   board has its own env — see [Supported boards](#supported-boards)).

That’s it. On boot the screen shows progress (WiFi → time sync → fetching), then
your glucose. Full flashing details, the ESP32-S3 BOOT/RST procedure, and Linux
serial-permission setup are in [`docs/FLASHING.md`](docs/FLASHING.md).

## Configuration

- **Credentials** → `include/secrets.h` (`WIFI_SSID`, `WIFI_PASS`, `NS_URL`, `NS_TOKEN`)
- **App settings** → [`src/core/config.h`](src/core/config.h): thresholds (mmol), poll
  interval, stale timeout, brightness levels, sparkline length, timezone.
- **Board geometry** (resolution/rotation/pins) → the active board's header,
  e.g. [`src/board/jc3248w535/pins.h`](src/board/jc3248w535/pins.h).

Default glucose thresholds (mmol/L): urgent-low `3.0`, low `3.9`, high `10.0`,
urgent-high `13.3`. Edit `src/core/config.h` to taste.

## Supported boards

The firmware splits into device-independent code and a thin per-board layer, so
new ESP32 display modules can be added without touching the backend or UI.

| Board | Display / Touch | Status |
|---|---|---|
| **Guition JC3248W535** | 3.5″ 320×480 AXS15231B QSPI / AXS15231B I²C | ✅ Verified (reference) |

**Adding a board is a great way to contribute.** A new board is one
`src/board/<name>/` directory + one PlatformIO env, with **zero changes** to the
backend or UI — see [`docs/ADDING_A_BOARD.md`](docs/ADDING_A_BOARD.md) for the
walkthrough. PRs are welcome; please only submit a board you've **verified on real
hardware** (untested boards stay in a separate *experimental* tier in the table
above, with the status spelled out).

## Project layout

```
platformio.ini           shared [env] + one [env:<board>] per board
boards/jc3248w535.json    PlatformIO board JSON (flash / OCTAL PSRAM / partitions)
include/lv_conf.h         LVGL 9 configuration
include/secrets.h         your credentials (git-ignored; copied from secrets.example.h)
src/
  main.cpp               thin entry: init HAL → view → app, then loop
  core/                  device-INDEPENDENT backend
    app.cpp              poll scheduling; builds the ViewModel
    nightscout.cpp       HTTPS fetch + JSON parse + NTP
    net.cpp              WiFi connect / reconnect
    glucose.h            units, zones, trend mapping
    viewmodel.h          backend → view contract
    config.h             app settings (thresholds, timing, brightness, …)
  view/                  LVGL presentation (reads a ViewModel)
    ui.cpp               screen (value / arrow / delta / age / alert / sparkline)
    fonts/               pre-generated LVGL fonts (.c)
  hal/                   the contract each board implements (display/touch/board.h)
  board/                 one directory per board — the only place a board adds code
    jc3248w535/          reference: QSPI panel + I²C touch + pins + metadata
    _template/           skeleton to copy for a new board
scripts/gen_fonts.sh     regenerate the fonts (needs Node + the .otf/.ttf)
docs/                    HARDWARE / FLASHING / NIGHTSCOUT / ADDING_A_BOARD
```

## Why these choices

The JC3248W535 uses an **AXS15231B over QSPI**, which `TFT_eSPI` and stock
`LovyanGFX` don’t drive. The verified-working path is **Arduino_GFX**
(`Arduino_ESP32QSPI` + `Arduino_AXS15231B` + `Arduino_Canvas`) with **LVGL 9**.
PSRAM is **octal** (`memory_type = qio_opi`) — a plain `esp32-s3-devkitc-1` board
leaves PSRAM off and the display init fails, which is why this repo ships a custom
board definition. See [`docs/HARDWARE.md`](docs/HARDWARE.md) for the receipts.

Arduino_GFX also drives a wide range of other controllers (ST7789, ILI9341,
ST7796, GC9A01, RGB/QSPI panels, …), which is what makes adding other boards
practical without swapping the graphics stack.

## Credits

- Nightscout data model & trend logic informed by
  [ireneusz-ptak/ns-thingy](https://github.com/ireneusz-ptak/ns-thingy).
- JC3248W535 display/touch bring-up cross-referenced from
  [byte-me404/JC3248W535_lvgl_test](https://github.com/byte-me404/JC3248W535_lvgl_test),
  [NorthernMan54/JC3248W535EN](https://github.com/NorthernMan54/JC3248W535EN) and
  [me-processware/JC3248W535-Driver](https://github.com/me-processware/JC3248W535-Driver).
- Inspiration: [SugarPixel](https://customtypeone.com/collections/sugarpixel),
  [SugarDash](https://mysugardash.com/).
- Libraries: [Arduino_GFX](https://github.com/moononournation/Arduino_GFX),
  [LVGL](https://lvgl.io), [ArduinoJson](https://arduinojson.org).

## License

[MIT](LICENSE).
