#pragma once
// ============================================================================
//  On-screen text. Norwegian by default — change it to whatever you like.
// ============================================================================
// Every word the display shows lives here, so you can translate the UI without
// touching the view code. Keep the "%d" in BSB_TXT_AGE_MIN — it's the number of
// minutes. The fonts ship with the glyphs these strings use; if you add letters
// with accents/diacritics, regenerate the fonts (scripts/gen_fonts.sh).

// ---- Status screen (shown before the first reading arrives) ----
#define BSB_TXT_BOOTING       "Starter..."
#define BSB_TXT_WIFI_CONNECT  "Kobler til WiFi..."
#define BSB_TXT_TIME_SYNC     "Synkroniserer tid..."
#define BSB_TXT_FETCHING      "Henter glukose..."
#define BSB_TXT_WIFI_FAILED   "WiFi feilet"
#define BSB_TXT_WAITING       "Venter på data"
#define BSB_TXT_WIFI_OFFLINE  "WiFi frakoblet"

// ---- Reading screen ----
#define BSB_TXT_NO_NET        "ingen nett"      // offline warning (top-left)
#define BSB_TXT_NOW           "nå"              // reading age: under a minute
#define BSB_TXT_AGE_MIN       "%d min siden"    // reading age: N minutes (keep %d)
