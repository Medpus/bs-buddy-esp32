#pragma once
// ============================================================================
//  User-tunable settings. WiFi/Nightscout credentials live in secrets.h.
// ============================================================================
#include "secrets.h"

// NB: Display geometry (resolution/rotation) is board-specific and lives in the
// active board's header (src/board/<board>/pins.h), not here. config.h is the
// device-INDEPENDENT app config: thresholds, timing, brightness, time sync.

// ---- Glucose thresholds (mmol/L) — used to color-code the value ----
// Defaults follow common consensus targets; edit to taste.
#define BSB_URGENT_LOW_MMOL    3.0f
#define BSB_LOW_MMOL           3.9f
#define BSB_HIGH_MMOL         10.0f
#define BSB_URGENT_HIGH_MMOL  13.3f

// ---- Timing ----
#define BSB_POLL_INTERVAL_MS   60000UL                 // normal Nightscout poll
#define BSB_RETRY_INTERVAL_MS  15000UL                 // faster retry after a failure
#define BSB_STALE_AFTER_MS    (13UL * 60UL * 1000UL)   // reading older than this = stale

// Number of recent readings to fetch for the sparkline (~5 min apart -> 96 ≈ 8 h).
#define BSB_HISTORY_N          96

// ---- Backlight brightness (0-100 %) ----
// Three modes cycled by tapping the screen: NORMAL -> LOW -> OFF -> NORMAL.
// NORMAL is the default always-on level (kept dim on purpose).
#define BSB_BRIGHTNESS_NORMAL  25
#define BSB_BRIGHTNESS_LOW      4
#define BSB_BRIGHTNESS_OFF      0

// ---- WiFi ----
#define BSB_WIFI_CONNECT_TIMEOUT_MS  20000UL

// ---- Time sync (for "minutes ago") ----
#define BSB_NTP_SERVER1  "pool.ntp.org"
#define BSB_NTP_SERVER2  "time.google.com"
// POSIX TZ string. Default: Europe/Oslo (CET/CEST). Only affects an optional clock;
// "minutes ago" works in UTC regardless.
#define BSB_TZ           "CET-1CEST,M3.5.0,M10.5.0/3"

#ifndef BSB_VERSION
#define BSB_VERSION "dev"
#endif
