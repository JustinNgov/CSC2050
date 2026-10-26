# Testing lab3.c

Run:

```sh
./test_main.sh
```

The script creates small 24-bit BMP inputs in `test_inputs/`, builds `lab3.c`, and writes outputs to `test_outputs/`.

It checks two identity rotations with different process counts and one 90-degree rotation output.

You can also run one case manually:

```sh
gcc -std=c11 -Wall -Wextra lab3.c -lm -o lab3
./lab3 test_inputs/gradient_5x4.bmp 0 3 test_outputs/gradient_identity_3proc.bmp
```
