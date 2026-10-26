#!/usr/bin/env python3
"""Create small deterministic 24-bit BMP inputs for lab3 tests."""

import os
import struct


def write_bmp(path, width, height, pixel_at):
    row_bytes = width * 3
    padding = (4 - (row_bytes % 4)) % 4
    stride = row_bytes + padding
    pixel_data_size = stride * height
    file_size = 54 + pixel_data_size

    file_header = struct.pack(
        "<HIHHI",
        0x4D42,
        file_size,
        0,
        0,
        54,
    )
    info_header = struct.pack(
        "<IIIHHIIIIII",
        40,
        width,
        height,
        1,
        24,
        0,
        pixel_data_size,
        2835,
        2835,
        0,
        0,
    )

    with open(path, "wb") as bmp:
        bmp.write(file_header)
        bmp.write(info_header)
        for y in range(height):
            for x in range(width):
                r, g, b = pixel_at(x, y)
                bmp.write(bytes((b, g, r)))
            bmp.write(b"\x00" * padding)


def gradient_pixel(x, y):
    return (
        (40 * x + 17 * y) % 256,
        (25 * x + 60 * y) % 256,
        (90 + 30 * x + 11 * y) % 256,
    )


def stripe_pixel(x, y):
    if (x + y) % 2 == 0:
        return (230, 40 + y * 20, 30 + x * 10)
    return (20 + y * 30, 170, 220 - x * 15)


def main():
    os.makedirs("test_inputs", exist_ok=True)
    write_bmp("test_inputs/gradient_5x4.bmp", 5, 4, gradient_pixel)
    write_bmp("test_inputs/stripe_8x5.bmp", 8, 5, stripe_pixel)


if __name__ == "__main__":
    main()
