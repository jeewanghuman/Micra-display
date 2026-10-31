#!/usr/bin/env bash
# Regenerate src/ui/fonts/font_outfit_<size>.c — the round glass UI's text
# faces (ui::font_dp returns them when ui::round_glass()). Outfit is SIL OFL
# 1.1 (src/ui/fonts/OFL-Outfit.txt). Each face falls back to the same-size
# built-in Montserrat, which carries LVGL's LV_SYMBOL_* icon glyphs.
#
# Needs: python3 + fonttools (pip install fonttools), node + npx (lv_font_conv
# is fetched on the fly). Run from the repo root.
set -euo pipefail
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

curl -sSLf -o "$WORK/Outfit.ttf" \
  "https://raw.githubusercontent.com/google/fonts/main/ofl/outfit/Outfit%5Bwght%5D.ttf"
# Freeze the variable font at Regular (400): reads cleanly at 14 px on the
# 235 ppi panel and pairs with the thin (150) hero numerals.
python3 -m fontTools.varLib.instancer "$WORK/Outfit.ttf" wght=400 \
  -o "$WORK/Outfit-400.ttf" -q
# Tabular figures: point 0-9 at Outfit's .tf glyphs (its 'tnum' feature,
# which lv_font_conv can't apply), so a changing number doesn't shift sideways
# — the proportional '1' is half the width of '0'.
python3 - "$WORK/Outfit-400.ttf" <<'PY'
import sys
from fontTools.ttLib import TTFont
p = sys.argv[1]
f = TTFont(p)
names = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"]
for t in f["cmap"].tables:
    if t.isUnicode():
        for i, n in enumerate(names):
            if 0x30 + i in t.cmap:
                t.cmap[0x30 + i] = n + ".tf"
f.save(p)
PY

# Printable ASCII plus the few typographic marks the UI strings use:
# ° · × – — ‘ ’ “ ” • … → ›
RANGE="0x20-0x7E,0xB0,0xB7,0xD7,0x2013-0x2014,0x2018-0x2019,0x201C-0x201D,0x2022,0x2026,0x2192,0x203A"
# The sizes ui::font_dp hands out on round (font_dp(12) rounds up to 14).
for SIZE in 14 16 20 24 28 36 48; do
  npx -y lv_font_conv --font "$WORK/Outfit-400.ttf" -r "$RANGE" --size "$SIZE" \
    --bpp 4 --format lvgl --no-compress --lv-font-name "font_outfit_$SIZE" \
    -o "src/ui/fonts/font_outfit_$SIZE.c"
  # Plain "lvgl.h" include (this tree's convention), a stable Opts line, and
  # the Montserrat fallback for the icon glyphs Outfit doesn't have.
  python3 - "$SIZE" "$RANGE" <<'PY'
import re, sys
size, rng = sys.argv[1], sys.argv[2]
p = f"src/ui/fonts/font_outfit_{size}.c"
s = open(p).read()
s = s.replace('''#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif''', f'''// Outfit (SIL Open Font License 1.1, see OFL-Outfit.txt beside this file),
// variable font instanced at wght=400, tabular digits, {size} px: round glass UI text
// (ui::font_dp). Falls back to lv_font_montserrat_{size} for LV_SYMBOL_*
// glyphs. Regenerate with tools/gen_ui_fonts.sh.
#include "lvgl.h"''', 1)
s = re.sub(r" \* Opts: .*", f" * Opts: --font Outfit-400.ttf -r {rng} --size {size} --bpp 4 "
           f"--format lvgl --no-compress --lv-font-name font_outfit_{size}", s, count=1)
assert s.count(".fallback = NULL,") == 1, p
s = s.replace(".fallback = NULL,", f".fallback = &lv_font_montserrat_{size},")
open(p, "w").write(s)
PY
  echo "wrote src/ui/fonts/font_outfit_$SIZE.c"
done

# Thin-line icons (Lucide, ISC — src/ui/fonts/LICENSE-Lucide.txt) for the
# round Home actions, the tab dock and the temperature dial. Only the glyphs
# include/ui/icons.h names; add a codepoint there AND here to use another.
(cd "$WORK" && npm pack lucide-static@1.61.0 --silent >/dev/null &&
   tar -xzf lucide-static-1.61.0.tgz)
# house settings chart-spline power droplet square weight bluetooth
# loader-circle minus plus x milk
ICONS="0xE0F5,0xE154,0xE60D,0xE140,0xE0B4,0xE167,0xE530,0xE05C,0xE10A,0xE11C,0xE13D,0xE1B2,0xE399"
for SIZE in 24 28 32; do
  npx -y lv_font_conv --font "$WORK/package/font/lucide.ttf" -r "$ICONS" --size "$SIZE" \
    --bpp 4 --format lvgl --no-compress --lv-font-name "font_lucide_$SIZE" \
    -o "src/ui/fonts/font_lucide_$SIZE.c"
  python3 - "$SIZE" "$ICONS" <<'PY'
import re, sys
size, icons = sys.argv[1], sys.argv[2]
p = f"src/ui/fonts/font_lucide_{size}.c"
s = open(p).read()
s = s.replace('''#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif''', f'''// Lucide icons (ISC, see LICENSE-Lucide.txt beside this file), {size} px:
// the glyphs include/ui/icons.h names. Regenerate with tools/gen_ui_fonts.sh.
#include "lvgl.h"''', 1)
s = re.sub(r" \* Opts: .*", f" * Opts: --font lucide.ttf -r {icons} --size {size} --bpp 4 "
           f"--format lvgl --no-compress --lv-font-name font_lucide_{size}", s, count=1)
open(p, "w").write(s)
PY
  echo "wrote src/ui/fonts/font_lucide_$SIZE.c"
done
cp "$WORK/package/LICENSE" src/ui/fonts/LICENSE-Lucide.txt
