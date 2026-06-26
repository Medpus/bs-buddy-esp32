#pragma once
// ============================================================================
//  View layer. Renders a ViewModel onto the LVGL screen. Knows nothing about
//  Nightscout, WiFi or the panel — it only reads a ViewModel and draws.
// ============================================================================
#include "viewmodel.h"   // core/ (via -I)
#include "board.h"       // hal/  (via -I) — screen geometry for layout

// Build the static UI once, given the active board's screen geometry.
void ui_build(const BoardInfo &board);

// Render one frame from the ViewModel (call ~1 Hz and after each fetch).
void ui_render(const ViewModel &vm);
