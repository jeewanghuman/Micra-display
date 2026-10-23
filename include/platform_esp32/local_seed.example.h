#pragma once

// Per-developer credential seed. Copy this file to local_seed.h (same
// directory — git-ignored, so the values never reach the repo) and fill in
// what you want baked into YOUR builds. Any line can be deleted.
//
// Config::begin() writes these into NVS on boot only when they CHANGE (a
// hash of the seed is kept in the "seedhash" key): flash a build with new
// values and they are applied once; after that the device's own settings win,
// so Forget / Set up WiFi on the screen keep working until the seed itself is
// edited again. Without local_seed.h nothing is compiled in.
//
// The values sit in the firmware image in plain text — don't hand that image
// to anyone else.

// The Micra: its Bluetooth MAC (as Settings > Micra > Bluetooth shows it
// after a scan), a display name, and the machine's Bluetooth token.
#define APOLLO_SEED_MICRA_MAC   "AA:BB:CC:DD:EE:FF"
#define APOLLO_SEED_MICRA_NAME  "MICRA_MR000000"
#define APOLLO_SEED_MICRA_TOKEN "your-token-here"

// Home WiFi (also switches WiFi on).
#define APOLLO_SEED_WIFI_SSID   "HomeWiFi"
#define APOLLO_SEED_WIFI_PASS   "password"
