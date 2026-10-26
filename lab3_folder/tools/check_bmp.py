#!/usr/bin/env python3
"""Minimal BMP checks used by test_main.sh."""

import os
import struct
import sys


def read_headers(path):
    with open(path, "rb") as bmp:
        header = bmp.read(54)

    if len(header) != 54:
        raise ValueError(f"{path}: expected at least 54 header bytes")

    bf_type, bf_size, _, _, bf_off_bits = struct.unpack("<HIHHI", header[:14])
    (
        bi_size,
        bi_width,
        bi_height,
        bi_planes,
        bi_bit_count,
        bi_compression,
        bi_size_image,
        _,
        _,
        _,
        _,
    ) = struct.unpack("<IIIHHIIIIII", header[14:54])

    if bf_type != 0x4D42:
        raise ValueError(f"{path}: not a BMP file")
    if bi_size != 40:
        raise ValueError(f"{path}: expected BITMAPINFOHEADER size 40")
    if bi_planes != 1:
        raise ValueError(f"{path}: expected one color plane")
    if bi_bit_count != 24:
        raise ValueError(f"{path}: expected 24-bit color")
    if bi_compression != 0:
        raise ValueError(f"{path}: expected an uncompressed BMP")

    actual_size = os.path.getsize(path)
    if bf_size != actual_size:
        raise ValueError(f"{path}: header size {bf_size} != actual size {actual_size}")
    if actual_size <= bf_off_bits:
        raise ValueError(f"{path}: missing pixel data")

    return bi_width, bi_height, bi_size_image


def same_shape(input_path, output_path):
    input_width, input_height, _ = read_headers(input_path)
    output_width, output_height, output_image_size = read_headers(output_path)

    if (input_width, input_height) != (output_width, output_height):
        raise ValueError(
            f"{output_path}: dimensions {output_width}x{output_height} "
            f"do not match {input_width}x{input_height}"
        )
    if output_image_size == 0:
        raise ValueError(f"{output_path}: image data size should not be zero")


def main():
    if len(sys.argv) != 3:
        print("usage: check_bmp.py INPUT_BMP OUTPUT_BMP", file=sys.stderr)
        return 2

    same_shape(sys.argv[1], sys.argv[2])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
