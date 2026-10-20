#pragma once

#include <lvgl.h>

// "Liquid glass" building blocks for the round-glass UI (the 3.4C). Plain
// LVGL styles, no backdrop blur — the panel can't afford a live blur, so the
// look is faked the way the design mockups describe it:
//   - a translucent white fill (flat — a faint gradient bands in RGB565),
//   - a thin white rim plus a brighter specular arc along the top edge,
//   - a soft drop shadow so the piece floats,
//   - drawn over the generated crema backdrop (crema_backdrop), which is what
//     gives the clear glass something to show.

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

// The crema backdrop: soft, out-of-focus amber / mocha / cream fields with a
// darker rim, w x h RGB565, ordered-dithered so the gradients don't band.
// Rendered ONCE on first call into a malloc'd buffer (w*h*2 bytes — on the
// ESP32-P4 a large malloc lands in PSRAM) and kept for the life of the app;
// later calls (theme rebuilds) return the same image. nullptr if the
// allocation fails: callers fall back to a flat backdrop color.
const lv_image_dsc_t* crema_backdrop(int w, int h);

// The backdrop's base color — the flat fallback, and what it fades to.
constexpr uint32_t kBackdropBase = 0x1A0F09;

}  // namespace ui::glass
