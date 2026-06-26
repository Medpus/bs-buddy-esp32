#!/usr/bin/env bash
# Regenerate the LVGL fonts under src/fonts/.
#
# The generated .c files are committed, so you only need this if you want to
# change sizes/glyphs. Requires Node (for npx lv_font_conv) and the source
# fonts below (adjust paths for your system).
#
#   ./scripts/gen_fonts.sh
set -euo pipefail

OUT="$(cd "$(dirname "$0")/.." && pwd)/src/fonts"
MONTSERRAT_BOLD="/usr/share/fonts/julietaula-montserrat-fonts/Montserrat-Bold.otf"
MONTSERRAT_TEXT="/usr/share/fonts/julietaula-montserrat-fonts/Montserrat-SemiBold.otf"
DEJAVU="/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf"

echo "Generating into $OUT"

# Big glucose value: digits, '.', '-', space
npx -y lv_font_conv@latest \
  --font "$MONTSERRAT_BOLD" --size 140 --bpp 4 --format lvgl --no-compress \
  --range '0x30-0x39,0x2E,0x2D,0x20' \
  --lv-font-name font_value_big -o "$OUT/font_value_big.c"

# Trend arrows
npx -y lv_font_conv@latest \
  --font "$DEJAVU" --size 96 --bpp 4 --format lvgl --no-compress \
  --range '0x2191,0x2192,0x2193,0x2194,0x2197,0x2198,0x003F' \
  --lv-font-name font_arrows -o "$OUT/font_arrows.c"

# UI text: ASCII + Norwegian (æøåÆØÅ) + degree
npx -y lv_font_conv@latest \
  --font "$MONTSERRAT_TEXT" --size 30 --bpp 4 --format lvgl --no-compress \
  --range '0x20-0x7F,0x00B0,0x00B7,0x00C5,0x00C6,0x00D8,0x00E5,0x00E6,0x00F8' \
  --lv-font-name font_text -o "$OUT/font_text.c"

echo "Done."
