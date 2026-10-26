#!/bin/sh
set -eu

cc="${CC:-gcc}"
out_dir="test_outputs"

python3 tools/make_test_bmps.py
mkdir -p "$out_dir"

"$cc" -std=c11 -Wall -Wextra lab2.c -lm -o lab2

./lab2 test_inputs/colors_3x2.bmp "$out_dir/colors_lightness.bmp" lightness 0.15
./lab2 test_inputs/colors_3x2.bmp "$out_dir/colors_contrast.bmp" contrast 1.4
./lab2 test_inputs/gradient_5x4.bmp "$out_dir/gradient_saturation.bmp" saturation 0.75

for output in \
    "$out_dir/colors_lightness.bmp" \
    "$out_dir/colors_contrast.bmp" \
    "$out_dir/gradient_saturation.bmp"
do
    if [ ! -s "$output" ]; then
        echo "Missing or empty output: $output" >&2
        exit 1
    fi
done

echo "Built ./lab2 and wrote BMP outputs in $out_dir/"
