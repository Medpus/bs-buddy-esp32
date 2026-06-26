#pragma once
// ============================================================================
//  BOARD TEMPLATE — copy this directory to src/board/<your-board>/ and fill in.
//  This header holds pins + display geometry for YOUR board. It is included only
//  by your board's own .cpp files (display/touch/board), never by shared code.
//
//  Reference implementation: src/board/jc3248w535/  (AXS15231B QSPI + I2C touch).
// ============================================================================

// --- Display geometry ---
//   rotation: 0=portrait, 1=landscape, 2=portrait flipped, 3=landscape flipped.
//   PANEL_*  = native panel resolution (before rotation).
//   SCREEN_* = logical resolution the UI sees (after rotation).
#define BOARD_PANEL_W   240     // TODO: native panel width
#define BOARD_PANEL_H   320     // TODO: native panel height
#define BOARD_ROTATION  0       // TODO
#define BOARD_SCREEN_W  240     // TODO: logical width  after rotation
#define BOARD_SCREEN_H  320     // TODO: logical height after rotation

// --- Display pins (example: generic SPI panel) ---
// #define PIN_TFT_BL    ...     // backlight
// #define PIN_TFT_CS    ...
// #define PIN_TFT_DC    ...
// #define PIN_TFT_SCK   ...
// #define PIN_TFT_MOSI  ...
// #define PIN_TFT_RST   ...

// --- Touch pins (omit entirely if the board has no touch) ---
// #define PIN_TOUCH_SDA  ...
// #define PIN_TOUCH_SCL  ...
// #define TOUCH_I2C_ADDR 0x..
