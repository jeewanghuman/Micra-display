#!/usr/bin/env bash
# Regenerate src/ui/fonts/font_outfit_hero.c — the large thin numerals of the
# round-glass Home screen. Outfit is SIL OFL 1.1 (src/ui/fonts/OFL-Outfit.txt).
#
# Needs: python3 + fonttools (pip install fonttools), node + npx (lv_font_conv
# is fetched on the fly). Run from the repo root.
set -euo pipefail
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

curl -sSLf -o "$WORK/Outfit.ttf" \
  "https://raw.githubusercontent.com/google/fonts/main/ofl/outfit/Outfit%5Bwght%5D.ttf"
# Freeze the variable font at weight 150 (between Thin and ExtraLight).
python3 -m fontTools.varLib.instancer "$WORK/Outfit.ttf" wght=150 \
  -o "$WORK/Outfit-150.ttf" -q
# Tabular figures: point 0-9 at Outfit's .tf glyphs (its 'tnum' feature,
# which lv_font_conv can't apply), so a changing number doesn't shift sideways
# — the proportional '1' is half the width of '0'.
python3 - "$WORK/Outfit-150.ttf" <<'PY'
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
# Digits, '.', '-', ':', 'g', '°' and space: "93.0°", "36.4 g", the idle clock "7:42", "--".
npx -y lv_font_conv --font "$WORK/Outfit-150.ttf" \
  -r 0x20,0x2D-0x2E,0x30-0x3A,0x67,0xB0 --size 150 --bpp 4 --format lvgl \
  --no-compress --lv-font-name font_outfit_hero \
  -o src/ui/fonts/font_outfit_hero.c
# lv_font_conv writes a conditional "lvgl/lvgl.h" include; this tree uses
# plain "lvgl.h", and its Opts line would carry the temp path.
python3 - <<'PY'
p = "src/ui/fonts/font_outfit_hero.c"
s = open(p).read()
s = s.replace('''#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif''', '''// Outfit (SIL Open Font License 1.1, see OFL-Outfit.txt beside this file),
// variable font instanced at wght=150, tabular digits, digits/'.'/'-'/':'/'g'/'°'
// only: the round Home's brew temperature and shot weight, and the idle clock.
// Regenerate with tools/gen_hero_font.sh.
#include "lvgl.h"''', 1)
import re
s = re.sub(r" \* Opts: .*", " * Opts: --font Outfit-150.ttf -r 0x20,0x2D-0x2E,0x30-0x3A,0x67,0xB0 "
           "--size 150 --bpp 4 --format lvgl --no-compress --lv-font-name font_outfit_hero", s, count=1)
open(p, "w").write(s)
PY
echo "wrote src/ui/fonts/font_outfit_hero.c"
