#!/usr/bin/env python3
import struct
from pathlib import Path


def write_bmp(path, width, height, pixels):
    row_size = width * 3
    padding = (4 - (row_size % 4)) % 4
    pixel_data_size = (row_size + padding) * height
    file_size = 54 + pixel_data_size

    file_header = struct.pack("<2sIHHI", b"BM", file_size, 0, 0, 54)
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

    with path.open("wb") as bmp:
        bmp.write(file_header)
        bmp.write(info_header)

        # BMP rows are stored bottom-up when height is positive.
        for row in reversed(pixels):
            for r, g, b in row:
                bmp.write(bytes((b, g, r)))
            bmp.write(b"\0" * padding)


def main():
    out_dir = Path("test_inputs")
    out_dir.mkdir(exist_ok=True)

    write_bmp(
        out_dir / "colors_3x2.bmp",
        3,
        2,
        [
            [(255, 0, 0), (0, 255, 0), (0, 0, 255)],
            [(0, 0, 0), (127, 127, 127), (255, 255, 255)],
        ],
    )

    write_bmp(
        out_dir / "gradient_5x4.bmp",
        5,
        4,
        [
            [(20, 30, 180), (55, 80, 190), (90, 130, 200), (130, 170, 210), (190, 210, 230)],
            [(180, 40, 40), (180, 80, 60), (180, 120, 80), (180, 160, 100), (180, 200, 120)],
            [(35, 140, 70), (70, 150, 90), (105, 160, 110), (140, 170, 130), (175, 180, 150)],
            [(20, 20, 20), (75, 75, 75), (125, 125, 125), (180, 180, 180), (235, 235, 235)],
        ],
    )

    print("Created test_inputs/colors_3x2.bmp")
    print("Created test_inputs/gradient_5x4.bmp")


if __name__ == "__main__":
    main()
