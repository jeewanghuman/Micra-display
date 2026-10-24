#pragma once

#include <lvgl.h>

// "Liquid glass" building blocks for the round-glass UI (the 3.4C). Plain
// LVGL styles, no backdrop blur — the panel can't afford a live blur, so the
// look is faked the way the design mockups describe it:
//   - a translucent white fill (flat — a faint gradient bands in RGB565),
//   - a thin white rim plus a brighter specular arc along the top edge,
//   - a soft drop shadow so the piece floats,
//   - drawn over a plain black backdrop (kBackdropBase).

namespace ui::glass {

enum class Tone : uint8_t {
  kClear,   // the default: nearly clear glass
  kBright,  // the selected / primary piece (Done, Set, the active tab)
  kAmber,   // the one warm call to action (Turn on, Connect, Stop)
};

// A glass button: an lv_button with every theme style removed and the glass
// styles applied, `radius` (LV_RADIUS_CIRCLE for round), the specular rim as
// a non-clickable child. Press feedback (ui::play_button_press) is wired.
lv_obj_t* make_button(lv_obj_t* parent, int radius, Tone tone);

// A non-interactive glass panel (capsules, the tab-bar dock): same styles,
// not clickable, not scrollable.
lv_obj_t* make_panel(lv_obj_t* parent, int radius, Tone tone);

// Re-apply a tone. Change-detecting (the tone is cached in the object's
// user_data), so the 2 Hz refresh path can call it every tick for free —
// and it restores the glass fill after a generic set_bg_color overwrote it.
void set_tone(lv_obj_t* obj, Tone tone);

// Turn a classic-layout subtree (Settings, Stats, modal cards) into dark
// glass over the black backdrop: theme-bg fills go transparent, card()/rail()
// fills become translucent (their colour kept, so the 2 Hz update paths that
// recolour buttons still work) with a faint white rim drawn as an outline (an
// outline sits outside the box, so no layout shifts). Idempotent and cheap —
// it only touches fills that are still opaque — so App re-runs it on each
// refresh to catch rows built later (scan results, the history list).
// Containers holding an lv_canvas (the flow graph) stay opaque: the canvas
// paints its own solid background.
void glassify_tree(lv_obj_t* root);

// The round UI's backdrop: true black. The glass pieces read as soft grey
// against it, and the arcs / amber accents carry the colour.
constexpr uint32_t kBackdropBase = 0x000000;

}  // namespace ui::glass
