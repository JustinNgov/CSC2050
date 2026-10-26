#!/bin/sh
set -eu

cd "$(dirname "$0")"

cc="${CC:-gcc}"
out_dir="test_outputs"

python3 tools/make_test_bmps.py
mkdir -p "$out_dir"

"$cc" -std=c11 -Wall -Wextra lab3.c -lm -o lab3

./lab3 test_inputs/gradient_5x4.bmp 0 1 "$out_dir/gradient_identity_1proc.bmp"
./lab3 test_inputs/gradient_5x4.bmp 0 3 "$out_dir/gradient_identity_3proc.bmp"

cmp -s test_inputs/gradient_5x4.bmp "$out_dir/gradient_identity_1proc.bmp"
cmp -s test_inputs/gradient_5x4.bmp "$out_dir/gradient_identity_3proc.bmp"

./lab3 test_inputs/stripe_8x5.bmp 1.5707963267948966 4 "$out_dir/stripe_rotate_90.bmp"
python3 tools/check_bmp.py test_inputs/stripe_8x5.bmp "$out_dir/stripe_rotate_90.bmp"

for output in \
    "$out_dir/gradient_identity_1proc.bmp" \
    "$out_dir/gradient_identity_3proc.bmp" \
    "$out_dir/stripe_rotate_90.bmp"
do
    if [ ! -s "$output" ]; then
        echo "Missing or empty output: $output" >&2
        exit 1
    fi
done

echo "Built ./lab3 and wrote BMP outputs in $out_dir/"
