# Testing lab2.c

Run:

```sh
./test_main.sh
```

The script creates two 24-bit BMP inputs in `test_inputs/`, builds `lab2.c`, and writes outputs to `test_outputs/`.

You can also run one case manually:

```sh
gcc -std=c11 -Wall -Wextra lab2.c -lm -o lab2
./lab2 test_inputs/colors_3x2.bmp test_outputs/colors_lightness.bmp lightness 0.15
```
