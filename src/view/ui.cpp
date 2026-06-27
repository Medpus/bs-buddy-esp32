#include "ui.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <lvgl.h>
#include "fonts.h"
#include "config.h"
#include "strings.h"

#define COL_BG      0x000000
#define COL_MUTED   0x8E8E93
#define COL_LIGHT   0xC7C7CC
#define COL_WARN    0xFF9F0A
#define COL_LINE    0xFFFFFF   // sparkline line
#define COL_TARGET  0x30D158   // low/high target lines (dashed, dimmed via opacity)

// Sparkline geometry (explicit so we can overlay the target lines)
#define CHART_X     12
#define CHART_Y     206
#define CHART_W     456
#define CHART_H     108
#define PAD_MGDL    18         // ~1 mmol headroom above/below the data

// Row (unit + delta·age) baseline, tucked right under the big value's digits.
#define ROW_Y       138

static lv_coord_t g_scrW = 480;
static lv_coord_t g_scrH = 320;

static lv_obj_t *lblClock  = nullptr;
static lv_obj_t *lblConn   = nullptr;  // connectivity warning (shown only when sustained)
static lv_obj_t *lblValue  = nullptr;
static lv_obj_t *lblArrow  = nullptr;
static lv_obj_t *lblUnit   = nullptr;
static lv_obj_t *lblFooter = nullptr;  // delta · min siden
static lv_obj_t *lblStatus = nullptr;  // centered message before first reading
static lv_obj_t *chart     = nullptr;
static lv_chart_series_t *series = nullptr;

// Dashed low/high target lines (drawn behind the sparkline)
static lv_obj_t *lineLow   = nullptr;
static lv_obj_t *lineHigh  = nullptr;
static lv_style_t s_dash;
static lv_point_precise_t s_hline[2] = {{0, 1}, {CHART_W, 1}};

static lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
  return l;
}

static void hide(lv_obj_t *o, bool h) {
  if (h) lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
  else   lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *make_dash_line(lv_obj_t *parent) {
  lv_obj_t *ln = lv_line_create(parent);
  lv_line_set_points(ln, s_hline, 2);
  lv_obj_set_size(ln, CHART_W, 3);   // explicit, so the thin line isn't clipped to 0 height
  lv_obj_add_style(ln, &s_dash, 0);
  lv_obj_remove_flag(ln, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(ln, LV_OBJ_FLAG_SCROLLABLE);
  return ln;
}

static void set_reading_mode(bool reading) {
  hide(lblValue,  !reading);
  hide(lblArrow,  !reading);
  hide(lblUnit,   !reading);
  hide(lblFooter, !reading);
  hide(chart,     !reading);
  hide(lineLow,   !reading);
  hide(lineHigh,  !reading);
  hide(lblStatus, reading);
  if (!reading) hide(lblConn, true);
}

void ui_build(const BoardInfo &board) {
  g_scrW = board.width;
  g_scrH = board.height;

  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);   // so set_pos() coords are absolute
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // Dashed-line style (shared by the low/high target lines)
  lv_style_init(&s_dash);
  lv_style_set_line_width(&s_dash, 2);
  lv_style_set_line_color(&s_dash, lv_color_hex(COL_TARGET));
  lv_style_set_line_opa(&s_dash, LV_OPA_50);
  lv_style_set_line_dash_width(&s_dash, 7);
  lv_style_set_line_dash_gap(&s_dash, 6);
  lv_style_set_line_rounded(&s_dash, false);

  // Top row
  lblClock = make_label(scr, &font_text, COL_LIGHT);
  lv_label_set_text(lblClock, "");
  lv_obj_align(lblClock, LV_ALIGN_TOP_RIGHT, -16, 8);

  lblConn = make_label(scr, &font_text, COL_WARN);
  lv_label_set_text(lblConn, BSB_TXT_NO_NET);
  lv_obj_align(lblConn, LV_ALIGN_TOP_LEFT, 16, 8);
  hide(lblConn, true);

  // Big value + inline trend arrow (upper-left)
  lblValue = make_label(scr, &font_value_big, 0xFFFFFF);
  lv_label_set_text(lblValue, "--");
  lv_obj_align(lblValue, LV_ALIGN_TOP_LEFT, 28, 36);

  lblArrow = make_label(scr, &font_arrows, 0xFFFFFF);
  lv_label_set_text(lblArrow, "?");
  lv_obj_align_to(lblArrow, lblValue, LV_ALIGN_OUT_RIGHT_MID, 16, 4);

  // Unit + footer on one row, tucked right under the digits
  lblUnit = make_label(scr, &font_text, COL_MUTED);
  lv_label_set_text(lblUnit, "mmol/L");
  lv_obj_align(lblUnit, LV_ALIGN_TOP_LEFT, 34, ROW_Y);

  lblFooter = make_label(scr, &font_text, COL_MUTED);
  lv_label_set_text(lblFooter, "");
  lv_obj_align(lblFooter, LV_ALIGN_TOP_RIGHT, -16, ROW_Y);

  // Dashed low/high target lines — created before the chart so the sparkline
  // draws on top of them.
  lineLow  = make_dash_line(scr);
  lineHigh = make_dash_line(scr);
  lv_obj_set_pos(lineLow,  CHART_X, CHART_Y + CHART_H);
  lv_obj_set_pos(lineHigh, CHART_X, CHART_Y);

  // Sparkline along the bottom
  chart = lv_chart_create(scr);
  lv_obj_set_pos(chart, CHART_X, CHART_Y);
  lv_obj_set_size(chart, CHART_W, CHART_H);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_div_line_count(chart, 0, 0);
  lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 40, 290);
  lv_chart_set_point_count(chart, BSB_HISTORY_N);
  lv_obj_set_style_bg_opa(chart, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(chart, 0, 0);
  lv_obj_set_style_pad_all(chart, 0, 0);
  lv_obj_set_style_line_width(chart, 3, LV_PART_ITEMS);   // line thickness
  lv_obj_set_style_width(chart, 0, LV_PART_INDICATOR);    // hide point markers
  lv_obj_set_style_height(chart, 0, LV_PART_INDICATOR);
  lv_obj_remove_flag(chart, LV_OBJ_FLAG_CLICKABLE);       // let taps bubble to screen
  lv_obj_remove_flag(chart, LV_OBJ_FLAG_SCROLLABLE);
  series = lv_chart_add_series(chart, lv_color_hex(COL_LINE), LV_CHART_AXIS_PRIMARY_Y);

  // Centered status message
  lblStatus = make_label(scr, &font_text, COL_LIGHT);
  lv_label_set_text(lblStatus, BSB_TXT_BOOTING);
  lv_obj_align(lblStatus, LV_ALIGN_CENTER, 0, 0);

  set_reading_mode(false);
}

// Refresh the sparkline + reposition the dashed target lines, with an adaptive
// Y-range that always includes the target corridor plus the data (with headroom).
static void render_history(const ViewModel &vm) {
  if (!chart || !series) return;
  if (vm.historyCount < 2) { hide(chart, true); hide(lineLow, true); hide(lineHigh, true); return; }
  hide(chart, false);
  hide(lineLow, false);
  hide(lineHigh, false);

  const int lowMg  = (int)(BSB_LOW_MMOL  * MGDL_PER_MMOL);   // ~70
  const int highMg = (int)(BSB_HIGH_MMOL * MGDL_PER_MMOL);   // ~180

  int dmin = 10000, dmax = 0;
  for (uint8_t i = 0; i < vm.historyCount; i++) {
    int v = vm.history[i];
    if (v <= 0) continue;
    if (v < dmin) dmin = v;
    if (v > dmax) dmax = v;
  }
  if (dmin > dmax) { dmin = lowMg; dmax = highMg; }
  int lo = (dmin < lowMg ? dmin : lowMg) - PAD_MGDL;
  int hi = (dmax > highMg ? dmax : highMg) + PAD_MGDL;
  if (lo < 0) lo = 0;
  if (hi - lo < 60) hi = lo + 60;
  lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, lo, hi);

  lv_chart_set_point_count(chart, vm.historyCount);
  for (uint8_t i = 0; i < vm.historyCount; i++) {
    lv_chart_set_series_value_by_id(chart, series, i, vm.history[i]);
  }
  lv_chart_refresh(chart);

  // Map the low/high thresholds to screen Y and move the dashed lines there.
  float span = (float)(hi - lo);
  int yHigh = CHART_Y + (int)((float)(hi - highMg) / span * CHART_H);
  int yLow  = CHART_Y + (int)((float)(hi - lowMg)  / span * CHART_H);
  if (yHigh < CHART_Y)            yHigh = CHART_Y;
  if (yLow  > CHART_Y + CHART_H)  yLow  = CHART_Y + CHART_H;
  lv_obj_set_pos(lineHigh, CHART_X, yHigh);
  lv_obj_set_pos(lineLow,  CHART_X, yLow);
}

static void render_status(const ViewModel &vm) {
  if (!lblStatus) return;
  set_reading_mode(false);
  lv_label_set_text(lblStatus, vm.statusMsg);
  hide(lblClock, true);
}

static void render_reading(const ViewModel &vm) {
  set_reading_mode(true);
  render_history(vm);

  Zone z = vm.zone;
  uint32_t col = vm.stale ? COL_MUTED : zoneColor(z);

  char buf[24];

  // Value
  snprintf(buf, sizeof(buf), "%.1f", vm.mmol);
  lv_label_set_text(lblValue, buf);
  lv_obj_set_style_text_color(lblValue, lv_color_hex(col), 0);
  lv_obj_align(lblValue, LV_ALIGN_TOP_LEFT, 28, 36);

  // Arrow
  lv_label_set_text(lblArrow, vm.stale ? "?" : vm.trendGlyph);
  lv_obj_set_style_text_color(lblArrow, lv_color_hex(col), 0);
  lv_obj_align_to(lblArrow, lblValue, LV_ALIGN_OUT_RIGHT_MID, 16, 4);

  // Footer: delta · age (on the same row as the unit, right-aligned)
  char ageStr[20] = {0};
  if (vm.ageValid) {
    if (vm.ageMin <= 0) strcpy(ageStr, BSB_TXT_NOW);
    else                snprintf(ageStr, sizeof(ageStr), BSB_TXT_AGE_MIN, vm.ageMin);
  }
  char delta[12] = {0};
  if (vm.haveDelta) snprintf(delta, sizeof(delta), "%+.1f", vm.deltaMmol);
  char foot[48];
  if (delta[0] && ageStr[0]) snprintf(foot, sizeof(foot), "%s  ·  %s", delta, ageStr);
  else if (delta[0])         snprintf(foot, sizeof(foot), "%s", delta);
  else if (ageStr[0])        snprintf(foot, sizeof(foot), "%s", ageStr);
  else                       foot[0] = '\0';
  lv_label_set_text(lblFooter, foot);
  lv_obj_align(lblFooter, LV_ALIGN_TOP_RIGHT, -16, ROW_Y);

  // Connectivity warning (debounced upstream)
  hide(lblConn, !vm.connProblem);

  // Clock
  if (vm.timeValid) {
    snprintf(buf, sizeof(buf), "%02d:%02d", vm.hh, vm.mm);
    lv_label_set_text(lblClock, buf);
    hide(lblClock, false);
  } else {
    hide(lblClock, true);
  }
}

void ui_render(const ViewModel &vm) {
  if (vm.screen == ViewModel::Screen::Reading) render_reading(vm);
  else                                         render_status(vm);
}
