#pragma once
#include <Arduino.h>
#include <string.h>
#include "config.h"

// Nightscout reports sgv in mg/dL regardless of site display units.
static const float MGDL_PER_MMOL = 18.01559f;

enum class Zone { UrgentLow, Low, InRange, High, UrgentHigh, Unknown };

struct Reading {
  bool    valid      = false;
  int     sgv        = 0;        // mg/dL (current)
  float   mmol       = 0.0f;     // current, mmol/L
  float   deltaMmol  = 0.0f;     // change vs previous reading
  bool    haveDelta  = false;
  char    direction[20] = {0};   // Nightscout trend string
  int64_t dateMs     = 0;        // reading epoch, ms (UTC)

  // Recent history for the sparkline (mg/dL), index 0 = oldest.
  int16_t history[BSB_HISTORY_N] = {0};
  uint8_t historyCount = 0;
};

inline float mgdlToMmol(int mgdl) { return mgdl / MGDL_PER_MMOL; }

inline Zone classify(float mmol) {
  if (mmol <= 0)                    return Zone::Unknown;
  if (mmol <  BSB_URGENT_LOW_MMOL)  return Zone::UrgentLow;
  if (mmol <  BSB_LOW_MMOL)         return Zone::Low;
  if (mmol <  BSB_HIGH_MMOL)        return Zone::InRange;
  if (mmol <  BSB_URGENT_HIGH_MMOL) return Zone::High;
  return Zone::UrgentHigh;
}

inline uint32_t zoneColor(Zone z) {
  switch (z) {
    case Zone::UrgentLow:  return 0xFF3B30; // red
    case Zone::Low:        return 0xFFD60A; // yellow
    case Zone::InRange:    return 0x30D158; // green
    case Zone::High:       return 0xFF9F0A; // orange
    case Zone::UrgentHigh: return 0xFF453A; // red
    default:               return 0x8E8E93; // grey
  }
}

inline const char* zoneLabel(Zone z) {
  switch (z) {
    case Zone::UrgentLow:  return "Veldig lav";
    case Zone::Low:        return "Lav";
    case Zone::InRange:    return "I mål";   // not shown — calm when in range
    case Zone::High:       return "Høy";
    case Zone::UrgentHigh: return "Veldig høy";
    default:               return "";
  }
}

// Map a Nightscout direction string to a glyph in font_arrows.
inline const char* trendArrow(const char* dir) {
  if (!dir || !dir[0])                 return "?";
  if (!strcmp(dir, "DoubleUp"))        return "↑↑"; // up up
  if (!strcmp(dir, "SingleUp"))        return "↑";       // up
  if (!strcmp(dir, "FortyFiveUp"))     return "↗";       // up-right
  if (!strcmp(dir, "Flat"))            return "→";       // right
  if (!strcmp(dir, "FortyFiveDown"))   return "↘";       // down-right
  if (!strcmp(dir, "SingleDown"))      return "↓";       // down
  if (!strcmp(dir, "DoubleDown"))      return "↓↓"; // down down
  return "?";
}
