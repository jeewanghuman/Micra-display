// Liquid-glass Home for round glass (the 3.4C, 800x800) with no scale paired.
// The design is the "Apollo Round — Liquid Glass" Home mockup: brew (amber)
// and steam (blue) as two arcs hugging the bezel, the brew temperature as
// large thin numerals, a status line above it, steam in a blue-dotted glass
// capsule, and round glass Standby / Flush buttons. The arcs carry no curved
// captions: lv_arclabel spaced small caps unevenly, and the colours already
// key them. The tab dock and the clock tray are App's (see App::build).
//
// Geometry is in fractions of the diameter D, so the layout follows the panel
// rather than a fixed 800.

#include <cstdio>

#include "ui/glass.h"
#include "ui/home_tab.h"
#include "ui/theme.h"
#include "ui/units.h"
#include "ui/widgets.h"

LV_FONT_DECLARE(font_outfit_hero);

namespace ui {
namespace {

constexpr uint32_t kBrewColor = 0xF2A24A;   // amber
constexpr uint32_t kSteamColor = 0xA9D2FF;  // pale blue
constexpr int kArcRange = 1000;             // arc values in permille

// One bezel arc. `r` is the stroke's centre radius; LVGL draws an arc inside
// its object's box, so the box is 2*(r + w/2) square, centred on the screen.
// Brew fills up the left side from 7 o'clock (NORMAL from 120 deg); steam
// fills up the right side from 5 o'clock (REVERSE from its end at 60 deg).
// With `bead`, the knob (a white dot at the fill's tip) is the "bead" of the
// mockup; without, the arc is the faint wide glow drawn underneath.
lv_obj_t* make_ring_arc(lv_obj_t* parent, int r, int width, bool left, uint32_t color,
                        lv_opa_t opa, bool track, bool bead) {
  lv_obj_t* arc = lv_arc_create(parent);
  lv_obj_remove_style_all(arc);
  lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  const int box = 2 * (r + width / 2);
  lv_obj_set_size(arc, box, box);
  lv_obj_center(arc);
  lv_arc_set_rotation(arc, 0);
  if (left) {
    lv_arc_set_bg_angles(arc, 120, 240);
    lv_arc_set_mode(arc, LV_ARC_MODE_NORMAL);
  } else {
    lv_arc_set_bg_angles(arc, 300, 60);
    lv_arc_set_mode(arc, LV_ARC_MODE_REVERSE);
  }
  lv_arc_set_range(arc, 0, kArcRange);
  lv_arc_set_value(arc, 0);

  lv_obj_set_style_arc_width(arc, width, LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(arc, true, LV_PART_MAIN);
  lv_obj_set_style_arc_color(arc, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_arc_opa(arc, track ? 36 : LV_OPA_TRANSP, LV_PART_MAIN);

  lv_obj_set_style_arc_width(arc, width, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(arc, opa, LV_PART_INDICATOR);

  if (bead) {
    lv_obj_set_style_bg_opa(arc, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_bg_color(arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_radius(arc, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(arc, ui::dp(4), LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc, ui::dp(18), LV_PART_KNOB);
    lv_obj_set_style_shadow_color(arc, lv_color_hex(color), LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, LV_OPA_80, LV_PART_KNOB);
  }
  return arc;
}

// A round glass action button with its glyph inside and its caption beneath,
// stacked in a column (the column is what hides when the button does — see
// update_round_home).
lv_obj_t* make_round_action(lv_obj_t* parent, int size, const char* glyph,
                            lv_obj_t** out_icon, lv_obj_t** out_caption) {
  lv_obj_t* col = lv_obj_create(parent);
  lv_obj_remove_style_all(col);
  lv_obj_remove_flag(col, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(col, ui::dp(10), 0);
  // Room for the button's drop shadow, which would otherwise be clipped.
  lv_obj_add_flag(col, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

  lv_obj_t* btn = ui::glass::make_button(col, LV_RADIUS_CIRCLE, ui::glass::Tone::kClear);
  lv_obj_set_size(btn, size, size);

  lv_obj_t* icon = lv_label_create(btn);
  lv_label_set_text(icon, glyph);
  lv_obj_set_style_text_font(icon, ui::font_dp(28), 0);
  lv_obj_center(icon);
  *out_icon = icon;

  lv_obj_t* cap = lv_label_create(col);
  lv_obj_set_style_text_font(cap, ui::font_dp(14), 0);
  lv_obj_set_style_text_color(cap, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_opa(cap, 215, 0);
  *out_caption = cap;
  return btn;
}

}  // namespace

void build_round_home(lv_obj_t* page, const ScreenProfile& screen, HomeWidgets& out) {
  const int d = screen.width < screen.height ? screen.width : screen.height;
  auto at = [d](int design_px) { return design_px * d / 800; };  // 800-px design
  out.round_glass = true;

  lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(page, 0, 0);
  lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);

  // --- Backdrop layer on the screen, behind everything (index 0) -----------
  lv_obj_t* scr = lv_obj_get_screen(page);
  lv_obj_t* back = lv_obj_create(scr);
  lv_obj_remove_style_all(back);
  lv_obj_remove_flag(back, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(back, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(back, screen.width, screen.height);
  lv_obj_set_pos(back, 0, 0);
  lv_obj_set_style_bg_color(back, lv_color_hex(ui::glass::kBackdropBase), 0);
  lv_obj_set_style_bg_opa(back, LV_OPA_COVER, 0);
  lv_obj_move_to_index(back, 0);
  out.rg_backdrop = back;

  if (const lv_image_dsc_t* img = ui::glass::crema_backdrop(screen.width, screen.height)) {
    lv_obj_t* bg = lv_image_create(back);
    lv_image_set_src(bg, img);
    lv_obj_set_pos(bg, 0, 0);
  }
  out.rg_dim = lv_obj_create(back);
  lv_obj_remove_style_all(out.rg_dim);
  lv_obj_remove_flag(out.rg_dim, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(out.rg_dim, screen.width, screen.height);
  lv_obj_set_style_bg_color(out.rg_dim, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(out.rg_dim, LV_OPA_TRANSP, 0);

  const int ring_r = at(372);
  out.rg_brew_glow = make_ring_arc(back, ring_r, at(26), true, kBrewColor, 60, false, false);
  out.rg_steam_glow = make_ring_arc(back, ring_r, at(26), false, kSteamColor, 60, false, false);
  out.rg_brew_arc = make_ring_arc(back, ring_r, at(10), true, kBrewColor, LV_OPA_COVER, true, true);
  out.rg_steam_arc = make_ring_arc(back, ring_r, at(10), false, kSteamColor, LV_OPA_COVER, true, true);

  // --- Readouts on the page ------------------------------------------------
  // Status: dot + word, above the numerals ("Ready", "Heating", ...).
  lv_obj_t* status = lv_obj_create(page);
  lv_obj_remove_style_all(status);
  lv_obj_remove_flag(status, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(status, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(status, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(status, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(status, at(10), 0);
  lv_obj_align(status, LV_ALIGN_TOP_MID, 0, at(178));
  out.micra_status_dot = lv_obj_create(status);
  lv_obj_remove_style_all(out.micra_status_dot);
  lv_obj_set_size(out.micra_status_dot, at(10), at(10));
  lv_obj_set_style_radius(out.micra_status_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(out.micra_status_dot, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(out.micra_status_dot, lv_color_hex(ui::theme::muted()), 0);
  out.micra_status_label = lv_label_create(status);
  lv_obj_set_style_text_font(out.micra_status_label, ui::font_dp(20), 0);
  lv_obj_set_style_text_color(out.micra_status_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_opa(out.micra_status_label, 230, 0);

  // Brew temperature: the hero numerals.
  out.brew_value = lv_label_create(page);
  lv_obj_set_style_text_font(out.brew_value, &font_outfit_hero, 0);
  lv_obj_set_style_text_color(out.brew_value, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_letter_space(out.brew_value, -at(4), 0);
  lv_label_set_text(out.brew_value, "--");
  lv_obj_align(out.brew_value, LV_ALIGN_TOP_MID, 0, at(222));

  // Steam: a glass capsule under the numerals.
  lv_obj_t* steam = ui::glass::make_panel(page, LV_RADIUS_CIRCLE, ui::glass::Tone::kClear);
  lv_obj_set_size(steam, at(220), at(48));
  lv_obj_align(steam, LV_ALIGN_TOP_MID, 0, at(362));
  lv_obj_set_flex_flow(steam, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(steam, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(steam, at(10), 0);
  lv_obj_t* sdot = lv_obj_create(steam);
  lv_obj_remove_style_all(sdot);
  lv_obj_set_size(sdot, at(8), at(8));
  lv_obj_set_style_radius(sdot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(sdot, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(sdot, lv_color_hex(kSteamColor), 0);
  out.boiler_value = lv_label_create(steam);
  lv_obj_set_style_text_font(out.boiler_value, ui::font_dp(20), 0);

  // Standby + Flush: round glass buttons, captions beneath. A flex row, so
  // when Flush is hidden (no drive line) Power re-centres on its own.
  lv_obj_t* row = lv_obj_create(page);
  lv_obj_remove_style_all(row);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(row, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, at(76), 0);
  lv_obj_align(row, LV_ALIGN_TOP_MID, 0, at(446));
  out.power_btn = make_round_action(row, at(80), LV_SYMBOL_POWER, &out.rg_power_icon,
                                    &out.power_label);
  out.flush_btn = make_round_action(row, at(80), LV_SYMBOL_TINT, &out.rg_flush_icon,
                                    &out.flush_label);
}

void update_round_home(HomeWidgets& w, const core::MachineSnapshot& state,
                       const core::BrewSnapshot& brew, bool fahrenheit, bool heating) {
  if (!w.round_glass) return;
  const bool connected = state.link == core::Link::Connected;
  const bool on = state.power == core::Power::On;

  // Numerals without the unit (the design's "93.0°"), steam as a sentence.
  char buf[24];
  if (connected) {
    std::snprintf(buf, sizeof(buf), "%.1f°",
                  static_cast<double>(ui::temp_disp(state.brew_temp_c, fahrenheit)));
  } else {
    std::snprintf(buf, sizeof(buf), "--");
  }
  ui::set_text(w.brew_value, buf);
  if (!connected) {
    std::snprintf(buf, sizeof(buf), "Steam --");
  } else if (state.steam_enabled) {
    std::snprintf(buf, sizeof(buf), "Steam %.0f°",
                  static_cast<double>(ui::temp_disp(state.boiler_temp_c, fahrenheit)));
  } else {
    std::snprintf(buf, sizeof(buf), "Steam off");
  }
  ui::set_text(w.boiler_value, buf);

  // Arcs: how far each boiler is from room temperature to its set point,
  // full when it has arrived. Empty when the link is down or steam is off.
  auto progress = [](float now, float target) {
    constexpr float kRoomC = 20.0f;
    if (target <= kRoomC + 1.0f) return 0;
    float f = (now - kRoomC) / (target - kRoomC);
    f = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
    return static_cast<int>(f * kArcRange + 0.5f);
  };
  const int brew_v = connected ? progress(state.brew_temp_c, state.brew_target_c) : 0;
  const int steam_v = (connected && state.steam_enabled)
                          ? progress(state.boiler_temp_c, state.boiler_target_c)
                          : 0;
  auto set_arc = [](lv_obj_t* arc, int v) {
    if (arc != nullptr && lv_arc_get_value(arc) != v) lv_arc_set_value(arc, v);
  };
  set_arc(w.rg_brew_arc, brew_v);
  set_arc(w.rg_brew_glow, brew_v);
  set_arc(w.rg_steam_arc, steam_v);
  set_arc(w.rg_steam_glow, steam_v);
  // A bead on an empty arc would sit alone at the start: hide it there.
  auto bead = [](lv_obj_t* arc, int v) {
    if (arc == nullptr) return;
    const lv_opa_t want = v > 0 ? LV_OPA_COVER : LV_OPA_TRANSP;
    if (lv_obj_get_style_bg_opa(arc, LV_PART_KNOB) != want) {
      lv_obj_set_style_bg_opa(arc, want, LV_PART_KNOB);
      lv_obj_set_style_shadow_opa(arc, v > 0 ? LV_OPA_80 : LV_OPA_TRANSP, LV_PART_KNOB);
    }
  };
  bead(w.rg_brew_arc, brew_v);
  bead(w.rg_steam_arc, steam_v);

  // The backdrop dims with the machine: full warmth when ready, a little
  // darker while heating, much darker asleep or offline.
  const lv_opa_t dim = !connected ? 150 : !on ? 140 : heating ? 60 : 0;
  if (w.rg_dim != nullptr && lv_obj_get_style_bg_opa(w.rg_dim, LV_PART_MAIN) != dim)
    lv_obj_set_style_bg_opa(w.rg_dim, dim, 0);

  // Power: update_home has already set the enabled state and a generic fill;
  // restore the glass and replace its "symbol  Label" text with glyph + word.
  using ui::glass::Tone;
  const bool pending = w.power_pending_until != 0;
  const char* glyph = LV_SYMBOL_POWER;
  const char* word = "Standby";
  Tone tone = Tone::kClear;
  switch (state.link) {
    case core::Link::Connected:
      if (pending) {
        glyph = LV_SYMBOL_REFRESH;
        word = "Working";
      } else if (!on) {
        word = "Turn on";
        tone = Tone::kAmber;
      }
      break;
    case core::Link::Disconnected:
      glyph = LV_SYMBOL_REFRESH;
      word = "Connect";
      tone = Tone::kAmber;
      break;
    case core::Link::NeedsToken:
    case core::Link::Unconfigured:
      glyph = LV_SYMBOL_SETTINGS;
      word = "Set up";
      tone = Tone::kAmber;
      break;
    case core::Link::Connecting:
    default:
      glyph = LV_SYMBOL_REFRESH;
      word = "Connecting";
      break;
  }
  ui::glass::set_tone(w.power_btn, tone);
  ui::set_text(w.rg_power_icon, glyph);
  ui::set_text(w.power_label, word);

  // Flush: update_home hides the button on boards without the drive line;
  // mirror that onto its column so the caption goes too and Power re-centres.
  // While a flush runs it turns amber and reads Stop.
  if (w.flush_btn != nullptr) {
    lv_obj_t* col = lv_obj_get_parent(w.flush_btn);
    const bool hidden = lv_obj_has_flag(w.flush_btn, LV_OBJ_FLAG_HIDDEN);
    if (hidden != lv_obj_has_flag(col, LV_OBJ_FLAG_HIDDEN)) {
      if (hidden) lv_obj_add_flag(col, LV_OBJ_FLAG_HIDDEN);
      else lv_obj_remove_flag(col, LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (w.flush_btn != nullptr && !lv_obj_has_flag(w.flush_btn, LV_OBJ_FLAG_HIDDEN)) {
    ui::glass::set_tone(w.flush_btn, brew.manual_flush ? Tone::kAmber : Tone::kClear);
    ui::set_text(w.rg_flush_icon, brew.manual_flush ? LV_SYMBOL_STOP : LV_SYMBOL_TINT);
    ui::set_text(w.flush_label, brew.manual_flush ? "Stop" : "Flush");
  }
}

}  // namespace ui
