#pragma once

// Describes the screen the UI is being built for, so a build can pick a layout
// without the UI hardcoding a resolution. We use a few DISCRETE size tiers (not
// continuous reflow):
//   compact — small panels like the 2-inch 320x240 (bottom tab bar)
//   wide    — landscape panels like the 4.3" 800x480 (side rail)
//   xl      — large landscape like the 7" 1024x600 (side rail, scaled up)
// plus a `round` flag for circular glass (the 3.4C's 800x800): the shape, not a
// size tier. Round screens lay every tab out in a centered band that fits
// inside the circle (round_band) and pick their tier from that band's size.
// plus a continuous `scale` on top of the tier: high-DPI panels (the 5" P4 at
// 1280x720) run an existing tier's LAYOUT with every pixel constant multiplied,
// so elements keep their physical size instead of shrinking with pixel density.

namespace ui {

struct ScreenProfile {
  int width;
  int height;
  // Proportional zoom applied to every layout constant (see ui::dp). The tier
  // checks below use the EFFECTIVE (pre-zoom) size, so 1280x720 @ 1.5 lays out
  // as the wide 800x480 tier, half again as large. 1.0 = classic behavior.
  float scale = 1.0f;
  // Circular glass: everything outside the inscribed circle is invisible. The
  // tier checks below then use the round_band, not the full square.
  bool round = false;
};

// The content rectangle on round glass: 0.8 x 0.6 of the diameter, centered.
// Its corners land exactly on the circle (0.4^2 + 0.3^2 = 0.5^2), so a
// rectangular layout placed here is fully visible — 640x480 on the 800 panel,
// the same class as the 4.3" layout. The caps above and below hold the status
// tray and the tab buttons. Physical px (not dp). Non-round: the whole screen.
struct Band {
  int x, y, w, h;
};
inline Band round_band(const ScreenProfile& screen) {
  if (!screen.round) return {0, 0, screen.width, screen.height};
  const int w = screen.width * 4 / 5;
  const int h = screen.height * 3 / 5;
  return {(screen.width - w) / 2, (screen.height - h) / 2, w, h};
}
// Size the layout is built for: the band on round glass, else the screen.
inline int layout_width(const ScreenProfile& screen) { return round_band(screen).w; }
inline int layout_height(const ScreenProfile& screen) { return round_band(screen).h; }

inline bool is_round(const ScreenProfile& screen) { return screen.round; }

// Global copy of the active profile's scale, so deep widget helpers don't need
// the ScreenProfile threaded through them. App::build (and the sim's render
// loop) set it before any widget is created.
inline float g_ui_scale = 1.0f;
inline void set_scale(float s) { g_ui_scale = (s > 0.0f) ? s : 1.0f; }
inline float scale() { return g_ui_scale; }
// Same for the profile's `round` flag: the glass styling deep inside a tab
// (settings rows, the shot graph's and the shot card's palettes) reads it
// instead of each file keeping its own copy. App::build sets it with the scale.
inline bool g_round_glass = false;
inline void set_round_glass(bool on) { g_round_glass = on; }
inline bool round_glass() { return g_round_glass; }
// Scale a pixel constant ("density-independent px"). Identity at scale 1.0.
inline int dp(int px) {
  return static_cast<int>(px * g_ui_scale + (px >= 0 ? 0.5f : -0.5f));
}

// Compact layout: bottom tab bar, tighter spacing, smaller fonts.
inline bool is_compact(const ScreenProfile& screen) {
  return layout_width(screen) / screen.scale < 480 ||
         layout_height(screen) / screen.scale < 360;
}

// XL layout: big landscape panels (7" 1024x600) — same structure as wide but
// scaled up (larger fonts, padding, controls).
inline bool is_xl(const ScreenProfile& screen) {
  return layout_width(screen) / screen.scale >= 960;
}

}  // namespace ui
