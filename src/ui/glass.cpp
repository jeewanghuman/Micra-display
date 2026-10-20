#include "ui/glass.h"

#include <cstdint>
#include <cstdlib>

#include "ui/screen.h"
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

const lv_image_dsc_t* crema_backdrop(int w, int h) {
  static lv_image_dsc_t dsc;
  static uint16_t* pixels = nullptr;
  if (pixels != nullptr) {
    return (static_cast<int>(dsc.header.w) == w && static_cast<int>(dsc.header.h) == h)
               ? &dsc
               : nullptr;
  }
  pixels = static_cast<uint16_t*>(std::malloc(static_cast<size_t>(w) * h * 2));
  if (pixels == nullptr) return nullptr;

  // Color fields, in 800-px design coordinates (scaled to w x h). Each is a
  // soft disc: weight = alpha * (1 - d^2/R^2)^2 inside R, composited in order
  // over the base — a cheap stand-in for the mockup's 40 px Gaussian blur.
  struct Blob {
    float cx, cy, r;
    uint32_t color;
    float alpha;
  };
  static constexpr Blob kBlobs[] = {
      {220.0f, 690.0f, 410.0f, 0xC8772F, 1.00f},  // caramel, lower left
      {680.0f, 150.0f, 360.0f, 0x5A2414, 1.00f},  // deep red-brown, upper right
      {260.0f, 190.0f, 290.0f, 0x2B160C, 1.00f},  // mocha shadow, upper left
      {710.0f, 470.0f, 220.0f, 0xA04E22, 0.85f},  // rust, right
      {540.0f, 620.0f, 230.0f, 0xF2CC94, 0.70f},  // crema highlight, lower right
  };
  // 4x4 ordered dither, so 565's 32/64 levels don't band on the soft fields.
  static constexpr uint8_t kBayer[4][4] = {
      {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

  const float sx = 800.0f / static_cast<float>(w);
  const float sy = 800.0f / static_cast<float>(h);
  const float base_r = static_cast<float>((kBackdropBase >> 16) & 0xFF);
  const float base_g = static_cast<float>((kBackdropBase >> 8) & 0xFF);
  const float base_b = static_cast<float>(kBackdropBase & 0xFF);
  for (int y = 0; y < h; ++y) {
    const float py = (static_cast<float>(y) + 0.5f) * sy;
    for (int x = 0; x < w; ++x) {
      const float px = (static_cast<float>(x) + 0.5f) * sx;
      float r = base_r, g = base_g, b = base_b;
      for (const Blob& bl : kBlobs) {
        const float dx = px - bl.cx, dy = py - bl.cy;
        const float q = (dx * dx + dy * dy) / (bl.r * bl.r);
        if (q >= 1.0f) continue;
        const float k = (1.0f - q) * (1.0f - q) * bl.alpha;
        r += (static_cast<float>((bl.color >> 16) & 0xFF) - r) * k;
        g += (static_cast<float>((bl.color >> 8) & 0xFF) - g) * k;
        b += (static_cast<float>(bl.color & 0xFF) - b) * k;
      }
      // Darken toward the rim of the glass so the arcs and edge text read.
      const float dx = px - 400.0f, dy = py - 400.0f;
      const float t = (dx * dx + dy * dy) / (400.0f * 400.0f);  // 0 centre .. 1 edge
      float vig = 1.0f;
      if (t > 0.25f) {
        const float u = (t - 0.25f) / 0.75f;
        vig = 1.0f - 0.5f * (u > 1.0f ? 1.0f : u * u * (3.0f - 2.0f * u));
      }
      const float d = (static_cast<float>(kBayer[y & 3][x & 3]) + 0.5f) / 16.0f;
      auto q5 = [&](float v) {
        int c = static_cast<int>(v * vig * 31.0f / 255.0f + d);
        return c < 0 ? 0 : (c > 31 ? 31 : c);
      };
      int g6 = static_cast<int>(g * vig * 63.0f / 255.0f + d);
      g6 = g6 < 0 ? 0 : (g6 > 63 ? 63 : g6);
      pixels[static_cast<size_t>(y) * w + x] =
          static_cast<uint16_t>((q5(r) << 11) | (g6 << 5) | q5(b));
    }
  }

  dsc = {};
  dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
  dsc.header.cf = LV_COLOR_FORMAT_RGB565;
  dsc.header.w = static_cast<uint32_t>(w);
  dsc.header.h = static_cast<uint32_t>(h);
  dsc.header.stride = static_cast<uint32_t>(w * 2);
  dsc.data_size = static_cast<uint32_t>(w * h * 2);
  dsc.data = reinterpret_cast<const uint8_t*>(pixels);
  return &dsc;
}

}  // namespace ui::glass
