from pathlib import Path
import struct

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
ASSET_DIR = ROOT / "fpga" / "assets"
OUTPUT_DIR = ROOT / "fpga" / "build" / "asset_test_data"
SCREEN_WIDTH, SCREEN_HEIGHT = 320, 240
TRANSPARENT = 0xF0F

ASSETS = {
    "chinabg": (600, 240, 4, 24008),
    "fredrikidle-comp": (560, 128, 5, 14950),
    "roundovertext": (272, 320, 1, 3628),
}

# name, src_x, src_y, width, height, dst_x, dst_y, z, flip_x
# Slots are deliberately out of z order.
LAYERS = [
    ("roundovertext", 0, 128, 272, 64, 24, 8, 3, False),
    ("fredrikidle-comp", 0, 0, 80, 128, 40, 104, 1, False),
    ("chinabg", 140, 0, 320, 240, 0, 0, 0, False),
    ("fredrikidle-comp", 0, 0, 80, 128, 201, 104, 2, True),
]


def load_asset(name, expected):
    raw = (ASSET_DIR / f"{name}.bin").read_bytes()
    width, height, index_bits = struct.unpack("<HHB", raw[:5])
    if (width, height, index_bits) != expected[:3]:
        raise ValueError(f"{name}: metadata differs from the mock ROM settings")
    colours = 1 << index_bits
    expected_bytes = 5 + (12 * colours + width * height * index_bits + 7) // 8
    if len(raw) != expected_bytes:
        raise ValueError(f"{name}: incorrect binary length")

    with Image.open(ASSET_DIR / f"{name}.png") as image:
        rgb = image.convert("RGB")
        if rgb.size != (width, height):
            raise ValueError(f"{name}: PNG dimensions differ from binary header")
        source = rgb.load()
        pixels = [
            ((source[x, y][0] >> 4) << 8)
            | ((source[x, y][1] >> 4) << 4)
            | (source[x, y][2] >> 4)
            for y in range(height)
            for x in range(width)
        ]

    palette_bytes = (12 * colours + 7) // 8
    packed = int.from_bytes(raw[5:5 + palette_bytes], "big")
    palette = [
        (packed >> (palette_bytes * 8 - 12 * (i + 1))) & 0xFFF
        for i in range(colours)
    ]
    bits = "".join(f"{byte:08b}" for byte in raw[5 + palette_bytes:])
    for i, pixel in enumerate(pixels):
        index = int(bits[i * index_bits:(i + 1) * index_bits], 2)
        if palette[index] != pixel:
            raise ValueError(f"{name}: binary pixel {i} differs from source PNG")

    payload = raw[5:]
    payload += bytes((-len(payload)) % 3)
    words = [
        int.from_bytes(payload[i:i + 3], "big")
        for i in range(0, len(payload), 3)
    ]
    if len(words) != expected[3]:
        raise ValueError(f"{name}: word count differs from the mock ROM settings")
    return words, pixels


def main():
    assets = {name: load_asset(name, settings) for name, settings in ASSETS.items()}
    frame = [0] * (SCREEN_WIDTH * SCREEN_HEIGHT)

    for name, sx, sy, width, height, dx, dy, z, flipped in sorted(
        LAYERS, key=lambda layer: layer[7]
    ):
        asset_width, asset_height = ASSETS[name][:2]
        if not (
            0 <= sx and 0 <= sy and sx + width <= asset_width
            and sy + height <= asset_height and 0 <= dx and 0 <= dy
            and dx + width <= SCREEN_WIDTH and dy + height <= SCREEN_HEIGHT
        ):
            raise ValueError(f"{name}: invalid test rectangle")
        pixels = assets[name][1]
        for y in range(height):
            for x in range(width):
                source_x = sx + (width - 1 - x if flipped else x)
                pixel = pixels[(sy + y) * asset_width + source_x]
                if pixel != TRANSPARENT:
                    frame[(dy + y) * SCREEN_WIDTH + dx + x] = pixel

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    for name, (words, pixels) in assets.items():
        output = OUTPUT_DIR / (name.replace("-", "_") + ".mem")
        output.write_text("".join(f"{word:06X}\n" for word in words), encoding="ascii")
        print(f"{name}: checked {len(pixels)} pixels, generated {len(words)} ROM words")
    (OUTPUT_DIR / "expected_frame.mem").write_text(
        "".join(f"{pixel:03X}\n" for pixel in frame), encoding="ascii"
    )
    print(f"Generated independent PNG reference frame in {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
