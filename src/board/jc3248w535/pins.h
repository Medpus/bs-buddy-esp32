#pragma once
// ============================================================================
//  Guition JC3248W535 board header (ESP32-S3-WROOM-1 N16R8).
//  Pins + display geometry for THIS board. Included only by this board's .cpp
//  files (display.cpp / touch.cpp / board.cpp) — not on the shared include path.
//  Verified against multiple independent sources — see docs/HARDWARE.md.
// ============================================================================

// --- Display geometry ---
// Native panel is 320x480 portrait. Hardware rotation is broken on this panel,
// so we rotate in SOFTWARE via the Arduino_Canvas rotation arg.
//   rotation: 0=portrait 320x480, 1=landscape 480x320, 2=portrait flipped,
//             3=landscape flipped.
#define BOARD_PANEL_W   320     // native panel width (portrait)
#define BOARD_PANEL_H   480     // native panel height
#define BOARD_ROTATION  3       // -> landscape, flipped 180° (case orientation)
#define BOARD_SCREEN_W  480     // logical width  after rotation
#define BOARD_SCREEN_H  320     // logical height after rotation

// --- Display: AXS15231B over QSPI (40 MHz) ---
#define PIN_TFT_BL    1     // backlight, active HIGH (PWM-capable)
#define PIN_TFT_CS    45
#define PIN_TFT_SCK   47
#define PIN_TFT_D0    21
#define PIN_TFT_D1    48
#define PIN_TFT_D2    40
#define PIN_TFT_D3    39
// Display has NO hardware reset pin (software reset over QSPI).

// --- Touch: AXS15231B capacitive, I2C @ 400 kHz ---
#define PIN_TOUCH_SDA  4
#define PIN_TOUCH_SCL  8    // NB: physically shared with the panel's (unused) DC line
#define PIN_TOUCH_INT  3    // disputed across board revisions; we poll, so it is unused
#define TOUCH_I2C_ADDR 0x3B

// --- Touch raw calibration (measured in portrait/rotation-0 frame) ---
#define TOUCH_RAW_X_MIN 12
#define TOUCH_RAW_X_MAX 310
#define TOUCH_RAW_Y_MIN 14
#define TOUCH_RAW_Y_MAX 461
