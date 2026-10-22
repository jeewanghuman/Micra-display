#include "ui/glass.h"

#include <cstdint>

#include "ui/screen.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace ui::glass {
namespace {

struct ToneStyle {
  uint32_t top;        // gradient start (top) color
  lv_opa_t top_opa;
  uint32_t bottom;     // gradient end (bottom) color
  lv_opa_t bottom_opa;
  lv_opa_t rim_opa;    // the thin all-round rim
};

ToneStyle tone_style(Tone tone) {
  switch (tone) {
    // Flat fills (top == bottom): a faint top-to-bottom gradient is what the
    // mockup draws, but at these opacities RGB565 can only step it in a few
    // visible bands. The specular top rim carries the "lit from above" cue.
    case Tone::kBright: return {0xFFFFFF, 72, 0xFFFFFF, 72, 130};
    case Tone::kAmber:  return {0xE8944A, 190, 0xE8944A, 190, 110};
    case Tone::kClear:
    default:            return {0xFFFFFF, 30, 0xFFFFFF, 30, 80};
  }
}

void apply_tone(lv_obj_t* obj, Tone tone) {
  const ToneStyle t = tone_style(tone);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(obj, lv_color_hex(t.top), 0);
  lv_obj_set_style_bg_main_opa(obj, t.top_opa, 0);
  lv_obj_set_style_bg_grad_color(obj, lv_color_hex(t.bottom), 0);
  lv_obj_set_style_bg_grad_opa(obj, t.bottom_opa, 0);
  lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_border_opa(obj, t.rim_opa, 0);
  lv_obj_set_user_data(obj, reinterpret_cast<void*>(static_cast<uintptr_t>(tone) + 1));
}

// Shared base: the shape, rim, shadow and the specular highlight child.
void style_base(lv_obj_t* obj, int radius, Tone tone) {
  lv_obj_remove_style_all(obj);
  lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(obj, radius, 0);
  lv_obj_set_style_border_width(obj, ui::dp(2), 0);
  lv_obj_set_style_border_color(obj, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_shadow_width(obj, ui::dp(26), 0);
  lv_obj_set_style_shadow_offset_y(obj, ui::dp(8), 0);
  lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_opa(obj, 55, 0);
  lv_obj_set_style_text_color(obj, lv_color_hex(0xFFFFFF), 0);
  apply_tone(obj, tone);

  // Specular rim: the top edge catches the light. A second, brighter border
  // drawn on the top side only, as a child so the parent keeps one style set.
  // Not clickable (taps fall through to the button) and out of any layout.
  lv_obj_t* spec = lv_obj_create(obj);
  lv_obj_remove_style_all(spec);
  lv_obj_add_flag(spec, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_remove_flag(spec, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(spec, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(spec, lv_pct(100), lv_pct(100));
  lv_obj_set_style_radius(spec, radius, 0);
  lv_obj_set_style_border_side(spec, LV_BORDER_SIDE_TOP, 0);
  lv_obj_set_style_border_width(spec, ui::dp(2), 0);
  lv_obj_set_style_border_color(spec, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_opa(spec, 220, 0);
}

void on_glass_pressed(lv_event_t*) { ui::play_button_press(); }

}  // namespace

lv_obj_t* make_button(lv_obj_t* parent, int radius, Tone tone) {
  lv_obj_t* btn = lv_button_create(parent);
  style_base(btn, radius, tone);
  // Pressed: the glass brightens under the finger.
  lv_obj_set_style_bg_main_opa(btn, 110, LV_STATE_PRESSED);
  lv_obj_set_style_bg_grad_opa(btn, 110, LV_STATE_PRESSED);
  lv_obj_set_style_opa(btn, LV_OPA_40, LV_STATE_DISABLED);
  lv_obj_add_event_cb(btn, on_glass_pressed, LV_EVENT_PRESSED, nullptr);
  return btn;
}

lv_obj_t* make_panel(lv_obj_t* parent, int radius, Tone tone) {
  lv_obj_t* panel = lv_obj_create(parent);
  style_base(panel, radius, tone);
  lv_obj_remove_flag(panel, LV_OBJ_FLAG_CLICKABLE);
  return panel;
}

void set_tone(lv_obj_t* obj, Tone tone) {
  if (obj == nullptr) return;
  const auto want = reinterpret_cast<void*>(static_cast<uintptr_t>(tone) + 1);
  // A generic set_bg_color (update_home's shared path) may have overwritten
  // the fill: re-apply whenever the color no longer matches the tone's top.
  const bool fill_ok =
      lv_color_to_u32(lv_obj_get_style_bg_color(obj, LV_PART_MAIN)) ==
      lv_color_to_u32(lv_color_hex(tone_style(tone).top));
  if (lv_obj_get_user_data(obj) == want && fill_ok) return;
  apply_tone(obj, tone);
}

namespace {

bool has_canvas_child(lv_obj_t* obj) {
  const uint32_t n = lv_obj_get_child_count(obj);
  for (uint32_t i = 0; i < n; ++i) {
    if (lv_obj_check_type(lv_obj_get_child(obj, static_cast<int32_t>(i)), &lv_canvas_class))
      return true;
  }
  return false;
}

void glassify_one(lv_obj_t* obj, uint32_t bg, uint32_t card, uint32_t rail) {
  if (lv_obj_get_style_bg_opa(obj, LV_PART_MAIN) != LV_OPA_COVER) return;
  const uint32_t c = lv_color_to_u32(lv_obj_get_style_bg_color(obj, LV_PART_MAIN)) & 0xFFFFFFu;
  if (c == bg) {  // page / menu backgrounds: let the backdrop through
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    return;
  }
  lv_opa_t opa;
  if (c == card) {
    opa = 150;
  } else if (c == rail) {
    opa = 175;
  } else {
    return;  // accent / status fills keep their full colour
  }
  if (has_canvas_child(obj)) return;
  lv_obj_set_style_bg_opa(obj, opa, 0);
  if (lv_obj_get_style_border_width(obj, LV_PART_MAIN) == 0 &&
      lv_obj_get_style_outline_width(obj, LV_PART_MAIN) == 0) {
    lv_obj_set_style_outline_width(obj, 1, 0);
    lv_obj_set_style_outline_color(obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_outline_opa(obj, 40, 0);
  }
}

void glassify_walk(lv_obj_t* obj, uint32_t bg, uint32_t card, uint32_t rail) {
  glassify_one(obj, bg, card, rail);
  const uint32_t n = lv_obj_get_child_count(obj);
  for (uint32_t i = 0; i < n; ++i)
    glassify_walk(lv_obj_get_child(obj, static_cast<int32_t>(i)), bg, card, rail);
}

}  // namespace

void glassify_tree(lv_obj_t* root) {
  if (root == nullptr) return;
  glassify_walk(root, ui::theme::bg() & 0xFFFFFFu, ui::theme::card() & 0xFFFFFFu,
                ui::theme::rail() & 0xFFFFFFu);
}

}  // namespace ui::glass
