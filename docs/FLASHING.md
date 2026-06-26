# Flashing & troubleshooting

This page covers the **reference board (JC3248W535)**. Other boards may use a
different USB bridge (e.g. a CH340/CP2102 UART instead of native USB) and a
different download procedure — check that board's `src/board/<board>/README.md`.
Select a board at build time with `pio run -e <board>` (the default is
`jc3248w535`).

The JC3248W535 uses the **ESP32-S3 native USB** (VID:PID `303A:1001`) — no
CH340/CP2102 driver needed.

## Normal flash

```bash
pio run -t upload
pio device monitor          # 115200 baud
```

With `ARDUINO_USB_CDC_ON_BOOT=1` (set in the board definition) the board
auto-resets into download mode, so no buttons are needed for a normal flash.

## If upload fails ("Connecting…", "No serial data received", wrong port)

Enter **download mode manually**:

1. Press and **hold BOOT**.
2. Press and release **RST** (a.k.a. EN) while still holding BOOT.
3. Release **BOOT**.

Then run `pio run -t upload` again. After flashing, press **RST** once to run the
new firmware.

If uploads still flake, lower the speed in `platformio.ini`:
`upload_speed = 460800` (or `115200`), and use a **direct USB port**, not a hub.

## Cable & port checklist

- Use a **USB-C data cable** — charge-only cables are the #1 "no port found" cause.
- Plug **directly** into the computer, not through a hub/dock.
- Confirm the board is seen: `lsusb | grep 303a` should show
  `Espressif USB JTAG/serial debug unit`.

## Linux: serial permission

The device appears as `/dev/ttyACM0` but is owned `root:dialout`. If you're not
in the `dialout` group it won't be writable. Fastest persistent fix — a udev rule:

```bash
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", MODE="0666"' \
  | sudo tee /etc/udev/rules.d/99-esp32-serial.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

(or `sudo usermod -aG dialout $USER` and re-login). A quick one-off is
`sudo chmod a+rw /dev/ttyACM0`, but that's lost on replug.

## Recovery / un-brick

The ROM bootloader can't be overwritten — you can always recover:

```bash
pio run -t erase        # full chip erase (clears bad firmware / partition table)
pio run -t upload
```

(Force download mode with BOOT+RST first if needed.)

## Common boot failures

| Symptom | Cause |
|---|---|
| Blank screen, serial says `gfx->begin() FAILED` | PSRAM not enabled as octal — check `memory_type = qio_opi` |
| Boot loop / `PSRAM ID read error` | Wrong PSRAM mode, or weak USB power (brownout) — use a solid port/cable |
| `invalid header: 0xffffffff` | Flash-size/partition mismatch — keep `flash_size = 16MB` |
| Wrong colors | Double byte-swap — don't call `lv_draw_sw_rgb565_swap` in the canvas path |
| No serial output | Built without USB CDC, or it crashed before USB init — re-flash via BOOT+RST |
