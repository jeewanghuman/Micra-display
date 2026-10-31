#pragma once

#include <lvgl.h>

// Thin-line icons for the round glass UI: Lucide glyphs (ISC, see
// src/ui/fonts/LICENSE-Lucide.txt) in three small LVGL fonts — 32 px for the
// Home action buttons, 28 px for the tab dock and the dial's -/+, 24 px for
// the dial's close. Each constant is the glyph's UTF-8; set it as a label's text
// with one of the fonts below. Only these glyphs are in the fonts: to add
// one, add its codepoint to ICONS in tools/gen_ui_fonts.sh and regenerate.
// Rectangular boards keep LVGL's LV_SYMBOL_* set.

LV_FONT_DECLARE(font_lucide_24);
LV_FONT_DECLARE(font_lucide_28);
LV_FONT_DECLARE(font_lucide_32);

namespace ui::icon {

inline constexpr const char kHome[] = "\xEE\x83\xB5";       // U+E0F5 house
inline constexpr const char kSettings[] = "\xEE\x85\x94";   // U+E154 settings (gear)
inline constexpr const char kStats[] = "\xEE\x98\x8D";      // U+E60D chart-spline
inline constexpr const char kPower[] = "\xEE\x85\x80";      // U+E140 power
inline constexpr const char kDroplet[] = "\xEE\x82\xB4";    // U+E0B4 droplet
inline constexpr const char kStop[] = "\xEE\x85\xA7";       // U+E167 square
inline constexpr const char kWeight[] = "\xEE\x94\xB0";     // U+E530 weight
inline constexpr const char kBluetooth[] = "\xEE\x81\x9C";  // U+E05C bluetooth
inline constexpr const char kBusy[] = "\xEE\x84\x8A";       // U+E10A loader-circle
inline constexpr const char kMinus[] = "\xEE\x84\x9C";      // U+E11C minus
inline constexpr const char kPlus[] = "\xEE\x84\xBD";       // U+E13D plus
inline constexpr const char kClose[] = "\xEE\x86\xB2";      // U+E1B2 x
inline constexpr const char kMilk[] = "\xEE\x8E\x99";       // U+E399 milk

}  // namespace ui::icon
