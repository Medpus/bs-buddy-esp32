#pragma once
#include "glucose.h"

void    ns_time_begin();          // start NTP sync (call once after WiFi is up)
int64_t ns_now_ms();              // current UTC epoch in ms, or 0 if not synced yet

// Fetch the most recent entries (BSB_HISTORY_N), fill `out` with the latest
// value, delta vs the previous reading, and the sparkline history.
// Returns true on success.
bool    ns_fetch(Reading &out);
