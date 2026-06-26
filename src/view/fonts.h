#pragma once
#include <lvgl.h>

// Custom fonts generated with lv_font_conv (see scripts/gen_fonts.sh).
// The .c files are committed so the project builds without Node.
#ifdef __cplusplus
extern "C" {
#endif
extern const lv_font_t font_value_big;  // Montserrat Bold 140 — digits, '.', '-'
extern const lv_font_t font_arrows;     // DejaVu Sans 96   — trend arrows
extern const lv_font_t font_text;       // Montserrat SemiBold 30 — Latin + æøåÆØÅ°
#ifdef __cplusplus
}
#endif
