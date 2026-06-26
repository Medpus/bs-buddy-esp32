// ============================================================================
//  BS-Buddy — secrets template
//
//  1. Copy this file to "secrets.h" (same folder):
//        cp include/secrets.example.h include/secrets.h
//  2. Fill in your WiFi and Nightscout details below.
//
//  secrets.h is git-ignored, so your credentials never get committed.
// ============================================================================
#pragma once

// --- WiFi (2.4 GHz only — the ESP32-S3 has no 5 GHz radio) ---
#define WIFI_SSID       "your-wifi-name"
#define WIFI_PASS       "your-wifi-password"

// --- Nightscout ---
// Base URL, no trailing slash. Must be reachable from your network.
#define NS_URL          "https://nightscout.example.com"

// Read-only access token. Leave as "" if your site is public.
// How to create one (recommended, read-only):
//   Nightscout -> Hamburger menu -> Admin Tools -> Subjects -> Add new Subject
//   Roles: "readable"  ->  Save  ->  copy the token (looks like "name-1a2b3c4d5e6f7a8b")
//   See docs/NIGHTSCOUT.md for details.
#define NS_TOKEN        ""
