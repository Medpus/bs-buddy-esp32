#include <Arduino.h>
#include <lvgl.h>

#include "display.h"   // hal/
#include "touch.h"     // hal/
#include "board.h"     // hal/
#include "ui.h"        // view/
#include "app.h"       // core/
#include "config.h"    // core/ (BSB_VERSION)

static void on_tap(lv_event_t *e) {
  LV_UNUSED(e);
  app_on_tap();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== BS-Buddy " BSB_VERSION " ===");

  if (!hal_display_init()) {
    Serial.println("FATAL: display init failed — continuing for serial diagnostics");
  }

  const BoardInfo &board = hal_board_info();
  Serial.printf("[board] %s (%ux%u, touch=%d)\n",
                board.name, board.width, board.height, board.hasTouch);

  ui_build(board);

  // Register an LVGL pointer device only if the board actually has touch.
  if (hal_touch_init()) {
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, hal_touch_read);
  }
  lv_obj_add_event_cb(lv_screen_active(), on_tap, LV_EVENT_CLICKED, nullptr);

  app_init();
}

void loop() {
  hal_display_loop();
  app_tick(millis());
  delay(2);
}
