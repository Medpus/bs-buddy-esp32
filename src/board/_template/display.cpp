// ============================================================================
//  BOARD TEMPLATE — display driver. Implements the HAL display contract
//  (src/hal/display.h) for your panel. Bring up ANY bus/controller here; the
//  rest of the firmware only sees hal_display_*().
//
//  The pattern below is the generic Arduino_GFX SPI path (ST7789 / ILI9341 /
//  ST7796 / GC9A01 …). For a QSPI panel, see the reference board
//  src/board/jc3248w535/display.cpp instead.
// ============================================================================
#include "display.h"   // hal/ (via -I)
#include <Arduino.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "pins.h"      // local geometry + pins
#include "config.h"    // core/ (via -I) — BSB_BRIGHTNESS_NORMAL

static Arduino_DataBus *bus      = nullptr;
static Arduino_GFX     *gfx      = nullptr;   // panel (or wrap in Arduino_Canvas if rotating in SW)
static lv_display_t    *disp     = nullptr;
static uint8_t         *draw_buf = nullptr;

static uint32_t millis_cb() { return millis(); }

static void flush_cb(lv_display_t *d, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  // TODO: many SPI panels need lv_draw_sw_rgb565_swap(px_map, w*h) here.
  //       The reference QSPI+Canvas board does NOT — verify on your hardware.
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  lv_display_flush_ready(d);
}

void hal_backlight_set(uint8_t pct) {
  if (pct > 100) pct = 100;
  // TODO: PWM on PIN_TFT_BL (see reference board), or plain on/off.
  (void)pct;
}

bool hal_display_init() {
  // TODO: construct your bus + panel, e.g.:
  //   bus = new Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_MOSI);
  //   gfx = new Arduino_ST7789(bus, PIN_TFT_RST, BOARD_ROTATION, true);
  // if (!gfx->begin()) return false;
  // gfx->fillScreen(0x0000);
  hal_backlight_set(BSB_BRIGHTNESS_NORMAL);

  lv_init();
  lv_tick_set_cb(millis_cb);

  uint32_t w = BOARD_SCREEN_W;
  uint32_t h = BOARD_SCREEN_H;
  uint32_t bufBytes = (w * h / 10) * 2;   // ~1/10 screen, RGB565
  draw_buf = (uint8_t *)heap_caps_malloc(bufBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!draw_buf) return false;

  disp = lv_display_create(w, h);
  lv_display_set_flush_cb(disp, flush_cb);
  lv_display_set_buffers(disp, draw_buf, NULL, bufBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
  return true;
}

void hal_display_loop() {
  lv_timer_handler();
  // TODO: if you render into an Arduino_Canvas, push it here: gfx->flush();
}
