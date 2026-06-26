#pragma once
// ============================================================================
//  ViewModel — the contract between the backend (core/) and the view (view/).
//  The backend fills one of these; the view only READS it. No LVGL, no pixels,
//  no board knowledge here. Swap the panel without touching this; swap the view
//  without touching the backend.
// ============================================================================
#include "glucose.h"

struct ViewModel {
  enum class Screen { Status, Reading };
  Screen screen = Screen::Status;

  // --- Status screen (boot / offline / waiting for first reading) ---
  char statusMsg[48] = {0};

  // --- Reading screen ---
  float       mmol        = 0.0f;
  Zone        zone        = Zone::Unknown;
  const char* trendGlyph  = "?";     // already mapped via trendArrow(); view applies stale
  bool        haveDelta   = false;
  float       deltaMmol   = 0.0f;
  bool        ageValid    = false;
  int         ageMin      = 0;
  bool        stale       = false;
  bool        connProblem = false;   // sustained connectivity problem (debounced)

  // --- Wall clock ---
  bool timeValid = false;
  int  hh = 0, mm = 0;

  // --- Sparkline history (mg/dL, index 0 = oldest) ---
  int16_t history[BSB_HISTORY_N] = {0};
  uint8_t historyCount = 0;
};
