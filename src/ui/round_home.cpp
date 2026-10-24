// Liquid-glass Home for round glass (the 3.4C, 800x800), following the
// "Apollo Round — Liquid Glass" mockups:
//
//   Idle  — brew (amber) and steam (blue) as arcs hugging the bezel, the brew
//           temperature as large thin numerals with a status line above,
//           steam in a glass capsule (tap: steam sheet), round glass Standby /
//           Flush buttons. With a scale: a weight capsule beside steam, a
//           Tare button, and the shot-mode pill.
//   Shot  — (scale paired; a shot running, settling or up for review) the
//           ring becomes progress toward the target weight, the weight is
//           the numerals, the timer and target under it, the flow graph in a
//           glass window, and the shot pill (Reset once it's done).
//
// The arcs carry no curved captions: lv_arclabel spaced small caps unevenly,
// and the colours already key them. The tab dock, the clock tray and the
// temperature dial / steam sheet are App's (see App::build).
//
// Geometry is in fractions of the diameter D (an 800-px design scaled by
// at()), so the layout follows the panel rather than a fixed 800.

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
constexpr uint32_t kShotColor = 0xFFD39B;   // light crema
constexpr int kArcRange = 1000;             // arc values in permille

// One bezel arc over [start, end] degrees (0 = 3 o'clock, clockwise). `r` is
// the stroke's centre radius; LVGL draws an arc inside its object's box, so
// the box is 2*(r + w/2) square, centred on the screen. REVERSE fills from
// the end angle back. With `bead`, the knob (a white dot at the fill's tip)
// is the "bead" of the mockup; without, the arc is the faint wide glow
// drawn underneath.
lv_obj_t* make_ring_arc(lv_obj_t* parent, int r, int width, int start, int end,
                        lv_arc_mode_t mode, uint32_t color, lv_opa_t opa, bool track,
                        bool bead) {
  lv_obj_t* arc = lv_arc_create(parent);
  lv_obj_remove_style_all(arc);
  lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  const int box = 2 * (r + width / 2);
  lv_obj_set_size(arc, box, box);
  lv_obj_center(arc);
  lv_arc_set_rotation(arc, 0);
  lv_arc_set_bg_angles(arc, start, end);
  lv_arc_set_mode(arc, mode);
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

// A full-page, transparent, non-scrolling container (the two Home views).
lv_obj_t* make_view(lv_obj_t* page) {
  lv_obj_t* v = lv_obj_create(page);
  lv_obj_remove_style_all(v);
  lv_obj_remove_flag(v, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(v, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(v, lv_pct(100), lv_pct(100));
  return v;
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
  if (out_icon != nullptr) *out_icon = icon;

  lv_obj_t* cap = lv_label_create(col);
  lv_obj_set_style_text_font(cap, ui::font_dp(14), 0);
  lv_obj_set_style_text_color(cap, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_opa(cap, 215, 0);
  if (out_caption != nullptr) *out_caption = cap;
  return btn;
}

// A glass capsule holding a coloured dot and a label (steam, weight).
lv_obj_t* make_dot_capsule(lv_obj_t* parent, int w, int h, uint32_t dot_color,
                           lv_obj_t** out_label) {
  lv_obj_t* cap = ui::glass::make_panel(parent, LV_RADIUS_CIRCLE, ui::glass::Tone::kClear);
  lv_obj_set_size(cap, w, h);
  lv_obj_set_flex_flow(cap, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cap, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(cap, ui::dp(10), 0);
  lv_obj_t* dot = lv_obj_create(cap);
  lv_obj_remove_style_all(dot);
  lv_obj_set_size(dot, ui::dp(8), ui::dp(8));
  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(dot, lv_color_hex(dot_color), 0);
  *out_label = lv_label_create(cap);
  lv_obj_set_style_text_font(*out_label, ui::font_dp(20), 0);
  return cap;
}

void set_hidden(lv_obj_t* obj, bool hidden) {
  if (obj == nullptr || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN) == hidden) return;
  if (hidden) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

void set_arc_value(lv_obj_t* arc, int v) {
  if (arc != nullptr && lv_arc_get_value(arc) != v) lv_arc_set_value(arc, v);
}

// A bead on an empty arc would sit alone at the start: hide it there.
void set_bead(lv_obj_t* arc, int v) {
  if (arc == nullptr) return;
  const lv_opa_t want = v > 0 ? LV_OPA_COVER : LV_OPA_TRANSP;
  if (lv_obj_get_style_bg_opa(arc, LV_PART_KNOB) != want) {
    lv_obj_set_style_bg_opa(arc, want, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, v > 0 ? LV_OPA_80 : LV_OPA_TRANSP, LV_PART_KNOB);
  }
}

}  // namespace

void build_round_home(lv_obj_t* page, const ScreenProfile& screen, bool scale_enabled,
                      HomeWidgets& out) {
  const int d = screen.width < screen.height ? screen.width : screen.height;
  auto at = [d](int design_px) { return design_px * d / 800; };  // 800-px design
  out.round_glass = true;
  out.scale_enabled = scale_enabled;

  lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(page, 0, 0);
  lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);

  // --- Backdrop layer on the screen, behind everything (index 0) -----------
  // Plain black on every tab (the other tabs glass over it); the arcs in
  // rg_ring are Home's only (App::sync_round_backdrop). Black, not an image:
  // the blurred crema fields this used to be read as muddy and busy behind
  // the numerals and the graph on the real panel.
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

  out.rg_ring = make_view(back);
  const int ring_r = at(372);
  out.rg_brew_glow = make_ring_arc(out.rg_ring, ring_r, at(26), 120, 240, LV_ARC_MODE_NORMAL,
                                   kBrewColor, 60, false, false);
  out.rg_steam_glow = make_ring_arc(out.rg_ring, ring_r, at(26), 300, 60, LV_ARC_MODE_REVERSE,
                                    kSteamColor, 60, false, false);
  out.rg_brew_arc = make_ring_arc(out.rg_ring, ring_r, at(10), 120, 240, LV_ARC_MODE_NORMAL,
                                  kBrewColor, LV_OPA_COVER, true, true);
  out.rg_steam_arc = make_ring_arc(out.rg_ring, ring_r, at(10), 300, 60, LV_ARC_MODE_REVERSE,
                                   kSteamColor, LV_OPA_COVER, true, true);
  if (scale_enabled) {
    // One 300 deg sweep from 7 o'clock round to 5 o'clock (the gap is the dock).
    out.rg_shot_glow = make_ring_arc(out.rg_ring, ring_r, at(30), 120, 60, LV_ARC_MODE_NORMAL,
                                     kShotColor, 60, false, false);
    out.rg_shot_arc = make_ring_arc(out.rg_ring, ring_r, at(12), 120, 60, LV_ARC_MODE_NORMAL,
                                    kShotColor, LV_OPA_COVER, true, true);
    lv_obj_add_flag(out.rg_shot_glow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(out.rg_shot_arc, LV_OBJ_FLAG_HIDDEN);
  }

  // Beads start hidden: an empty arc shows only its track.
  set_bead(out.rg_brew_arc, 0);
  set_bead(out.rg_steam_arc, 0);
  set_bead(out.rg_shot_arc, 0);

  // --- Idle view -----------------------------------------------------------
  lv_obj_t* idle = make_view(page);
  out.rg_idle_view = idle;

  // Status: dot + word, above the numerals ("Ready", "Heating", ...).
  lv_obj_t* status = lv_obj_create(idle);
  lv_obj_remove_style_all(status);
  lv_obj_remove_flag(status, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(status, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(status, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(status, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(status, at(10), 0);
  lv_obj_align(status, LV_ALIGN_TOP_MID, 0, at(scale_enabled ? 158 : 178));
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

  // Brew temperature: the numerals. Tap -> the temperature dial (App).
  out.brew_value = lv_label_create(idle);
  lv_obj_set_style_text_font(out.brew_value, &font_outfit_hero, 0);
  lv_obj_set_style_text_color(out.brew_value, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_letter_space(out.brew_value, -at(4), 0);
  lv_label_set_text(out.brew_value, "--");
  lv_obj_align(out.brew_value, LV_ALIGN_TOP_MID, 0, at(scale_enabled ? 196 : 222));
  lv_obj_add_flag(out.brew_value, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(out.brew_value, at(20));

  // Steam (tap -> steam sheet), and with a scale the weight beside it.
  lv_obj_t* caps = lv_obj_create(idle);
  lv_obj_remove_style_all(caps);
  lv_obj_remove_flag(caps, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(caps, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  lv_obj_set_size(caps, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(caps, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(caps, at(14), 0);
  lv_obj_align(caps, LV_ALIGN_TOP_MID, 0, at(scale_enabled ? 336 : 362));
  out.rg_steam_capsule = make_dot_capsule(caps, at(scale_enabled ? 196 : 220), at(48),
                                          kSteamColor, &out.boiler_value);
  lv_obj_add_flag(out.rg_steam_capsule, LV_OBJ_FLAG_CLICKABLE);
  if (scale_enabled) {
    make_dot_capsule(caps, at(196), at(48), kShotColor, &out.rg_weight_small);
  }

  // Standby + Flush (+ Tare): round glass buttons, captions beneath. A flex
  // row, so a hidden Flush (no drive line) lets the rest re-centre.
  lv_obj_t* row = lv_obj_create(idle);
  lv_obj_remove_style_all(row);
  lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(row, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, at(scale_enabled ? 52 : 76), 0);
  lv_obj_align(row, LV_ALIGN_TOP_MID, 0, at(scale_enabled ? 412 : 446));
  out.power_btn = make_round_action(row, at(80), LV_SYMBOL_POWER, &out.rg_power_icon,
                                    &out.power_label);
  out.flush_btn = make_round_action(row, at(80), LV_SYMBOL_TINT, &out.rg_flush_icon,
                                    &out.flush_label);
  if (scale_enabled) {
    out.tare_btn = make_round_action(row, at(80), LV_SYMBOL_LOOP, nullptr, &out.tare_label);
    lv_label_set_text(out.tare_label, "Tare");
  }

  if (!scale_enabled) return;

  // --- Shot pill: shot mode while idle (Auto shot / Shot detect / Manual),
  //     disabled mid-shot, Reset during review. Lives on the page, outside
  //     both views; update_round_home moves it between their slots. ---------
  out.shot_btn = ui::glass::make_button(page, LV_RADIUS_CIRCLE, ui::glass::Tone::kClear);
  lv_obj_set_size(out.shot_btn, at(210), at(54));
  out.shot_btn_label = lv_label_create(out.shot_btn);
  lv_obj_set_style_text_font(out.shot_btn_label, ui::font_dp(20), 0);
  lv_label_set_text(out.shot_btn_label, "Auto shot");
  lv_obj_center(out.shot_btn_label);
  lv_obj_align(out.shot_btn, LV_ALIGN_TOP_MID, 0, at(560));

  // --- Shot view -------------------------------------------------------------
  lv_obj_t* shot = make_view(page);
  out.rg_shot_view = shot;

  out.rg_shot_status = lv_label_create(shot);
  lv_obj_set_style_text_font(out.rg_shot_status, ui::font_dp(20), 0);
  lv_obj_set_style_text_color(out.rg_shot_status, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_opa(out.rg_shot_status, 230, 0);
  lv_obj_align(out.rg_shot_status, LV_ALIGN_TOP_MID, 0, at(150));

  out.scale_weight = lv_label_create(shot);
  lv_obj_set_style_text_font(out.scale_weight, &font_outfit_hero, 0);
  lv_obj_set_style_text_color(out.scale_weight, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_letter_space(out.scale_weight, -at(4), 0);
  lv_label_set_text(out.scale_weight, "-- g");
  lv_obj_align(out.scale_weight, LV_ALIGN_TOP_MID, 0, at(184));

  lv_obj_t* sub = lv_obj_create(shot);
  lv_obj_remove_style_all(sub);
  lv_obj_remove_flag(sub, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(sub, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(sub, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(sub, at(18), 0);
  lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, at(304));
  out.shot_timer_label = lv_label_create(sub);
  out.rg_shot_target = lv_label_create(sub);
  for (lv_obj_t* l : {out.shot_timer_label, out.rg_shot_target}) {
    lv_obj_set_style_text_font(l, ui::font_dp(24), 0);
    lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
  }
  lv_obj_set_style_text_opa(out.rg_shot_target, 190, 0);
  lv_label_set_text(out.shot_timer_label, "0.0 s");

  // The flow graph in a glass-rimmed window. Its canvas paints a solid
  // background, so the window itself stays opaque (glassify skips it).
  // A wide lens across the circle's fattest part: at y 348..528 the chord is
  // over 700 px, so 600 x 180 keeps every corner well inside the glass. The
  // canvas (its content box, 496 x 124 x 2 B = 120 KB) comes out of LVGL's
  // fixed pool; the rectangular boards' run from 170 KB to ~680 KB.
  lv_obj_t* graph = build_round_flow_graph(shot, at(600), at(180), out);
  lv_obj_align(graph, LV_ALIGN_TOP_MID, 0, at(348));
  lv_obj_set_style_radius(graph, at(56), 0);
  lv_obj_set_style_clip_corner(graph, true, 0);
  lv_obj_set_style_outline_width(graph, ui::dp(2), 0);
  lv_obj_set_style_outline_color(graph, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_outline_opa(graph, 70, 0);
  // Hidden only now: the graph had to be laid out (measured) visible.
  lv_obj_add_flag(shot, LV_OBJ_FLAG_HIDDEN);
}

void round_shot_progress(HomeWidgets& w, float shown_g, float target_g) {
  if (!w.round_glass || w.rg_shot_arc == nullptr) return;
  int v = 0;
  if (target_g > 0.1f && shown_g > 0.0f) {
    const float f = shown_g / target_g;
    v = static_cast<int>((f > 1.0f ? 1.0f : f) * kArcRange + 0.5f);
  }
  set_arc_value(w.rg_shot_arc, v);
  set_arc_value(w.rg_shot_glow, v);
  set_bead(w.rg_shot_arc, v);
}

void update_round_home(HomeWidgets& w, const core::MachineSnapshot& state,
                       const core::ScaleSnapshot& scale, const core::BrewSnapshot& brew,
                       bool fahrenheit) {
  if (!w.round_glass) return;
  const bool connected = state.link == core::Link::Connected;
  const bool on = state.power == core::Power::On;
  const bool shot_view = w.scale_enabled && brew.phase != core::ShotPhase::kIdle;

  // --- Which view ----------------------------------------------------------
  set_hidden(w.rg_idle_view, shot_view);
  set_hidden(w.rg_shot_view, !shot_view);
  set_hidden(w.rg_brew_arc, shot_view);
  set_hidden(w.rg_brew_glow, shot_view);
  set_hidden(w.rg_steam_arc, shot_view);
  set_hidden(w.rg_steam_glow, shot_view);
  set_hidden(w.rg_shot_arc, !shot_view);
  set_hidden(w.rg_shot_glow, !shot_view);
  if (w.shot_btn != nullptr) {
    const int d = lv_obj_get_width(lv_obj_get_screen(w.shot_btn));
    const int y = (shot_view ? 544 : 560) * d / 800;
    if (lv_obj_get_style_align(w.shot_btn, LV_PART_MAIN) != LV_ALIGN_TOP_MID ||
        lv_obj_get_style_y(w.shot_btn, LV_PART_MAIN) != y)
      lv_obj_align(w.shot_btn, LV_ALIGN_TOP_MID, 0, y);
  }

  // --- Idle readouts -------------------------------------------------------
  // Numerals without the unit (the design's "93.0°"), steam as a sentence.
  char buf[32];
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
  if (w.rg_weight_small != nullptr) {
    if (scale.connected) std::snprintf(buf, sizeof(buf), "%.1f g", static_cast<double>(scale.weight_g));
    else std::snprintf(buf, sizeof(buf), "Scale off");
    ui::set_text(w.rg_weight_small, buf);
  }

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
  set_arc_value(w.rg_brew_arc, brew_v);
  set_arc_value(w.rg_brew_glow, brew_v);
  set_arc_value(w.rg_steam_arc, steam_v);
  set_arc_value(w.rg_steam_glow, steam_v);
  set_bead(w.rg_brew_arc, brew_v);
  set_bead(w.rg_steam_arc, steam_v);

  // --- Shot readouts -------------------------------------------------------
  if (shot_view) {
    const char* mode = brew.mode == core::ShotMode::kAuto     ? "Auto shot"
                       : brew.mode == core::ShotMode::kDetect ? "Shot detect"
                                                              : "Manual";
    switch (brew.phase) {
      case core::ShotPhase::kBrewing:
        std::snprintf(buf, sizeof(buf), "Brewing (%s)", mode);  // no "·" in Montserrat
        break;
      case core::ShotPhase::kSettling:
        std::snprintf(buf, sizeof(buf), "Finishing");
        break;
      case core::ShotPhase::kReview:
      default:
        std::snprintf(buf, sizeof(buf), "Shot complete");
        break;
    }
    ui::set_text(w.rg_shot_status, buf);
    if (brew.target_weight_g > 0.1f)
      std::snprintf(buf, sizeof(buf), "of %.0f g", static_cast<double>(brew.target_weight_g));
    else
      buf[0] = '\0';
    ui::set_text(w.rg_shot_target, buf);
    // The 2 Hz path keeps the ring honest when the scale goes quiet; the fast
    // path (App::pump_scale_chart -> round_shot_progress) makes it smooth.
    if (brew.phase != core::ShotPhase::kReview && scale.connected) {
      const float shown =
          scale.weight_g - (core::unwired_net_weight(brew) ? brew.start_weight_g : 0.0f);
      round_shot_progress(w, shown, brew.target_weight_g);
    }
  }

  // --- Power: update_home has set the enabled state; draw glass + words. ----
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
  // mirror that onto its column so the caption goes too and the row
  // re-centres. While a flush runs it turns amber and reads Stop.
  if (w.flush_btn != nullptr) {
    lv_obj_t* col = lv_obj_get_parent(w.flush_btn);
    set_hidden(col, lv_obj_has_flag(w.flush_btn, LV_OBJ_FLAG_HIDDEN));
    if (!lv_obj_has_flag(w.flush_btn, LV_OBJ_FLAG_HIDDEN)) {
      ui::glass::set_tone(w.flush_btn, brew.manual_flush ? Tone::kAmber : Tone::kClear);
      ui::set_text(w.rg_flush_icon, brew.manual_flush ? LV_SYMBOL_STOP : LV_SYMBOL_TINT);
      ui::set_text(w.flush_label, brew.manual_flush ? "Stop" : "Flush");
    }
  }

  // Shot pill: update_home writes its words and an accent/warn rim + text
  // colour; keep the glass fill under them (Reset reads as the call to act).
  // (Not while a flash animation owns the fill — see set_shot_flash_fill.)
  if (w.shot_btn != nullptr && w.shot_flash_count <= 0 && w.stop_flash_count <= 0) {
    ui::glass::set_tone(w.shot_btn, brew.phase == core::ShotPhase::kReview ? Tone::kBright
                                                                           : Tone::kClear);
  }
}

}  // namespace ui
