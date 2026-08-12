#!/usr/bin/env python3
"""PNG -> PAA (DXT1 / DXT5) конвертер для UI-текстур DayZ.

Контейнер PAA воспроизведён 1:1 с рабочего файла из JobsMod_Client
(DogStay/modik4): magic, пустой блок тэгов, один мип-уровень, терминатор.

    uint16  type        0xFF01 = DXT1, 0xFF05 = DXT5
    uint16  0           тэгов/палитры нет
    uint16  width
    uint16  height
    uint24  data_len
    bytes   data        блоки DXT
    uint16  0           терминатор списка мипов

Мип-уровни намеренно не генерируются: для UI-текстур они выключаются
(правило экспорта из дизайн-хендоффа).

Использование:
    python3 tools/png_to_paa.py in.png out.paa [--size 2048x1024] [--dxt5]
"""

import argparse
import struct
import sys

import numpy as np
from PIL import Image

PAA_DXT1 = 0xFF01
PAA_DXT5 = 0xFF05


def to_blocks(a):
    """(H, W, C) -> (nblocks, 16, C), порядок блоков построчный."""
    h, w, c = a.shape
    a = a.reshape(h // 4, 4, w // 4, 4, c)
    a = a.transpose(0, 2, 1, 3, 4)
    return a.reshape(-1, 16, c)


def pack565(rgb):
    r = (rgb[..., 0].astype(np.uint16) >> 3) & 0x1F
    g = (rgb[..., 1].astype(np.uint16) >> 2) & 0x3F
    b = (rgb[..., 2].astype(np.uint16) >> 3) & 0x1F
    return (r << 11) | (g << 5) | b


def unpack565(v):
    r = ((v >> 11) & 0x1F).astype(np.uint16)
    g = ((v >> 5) & 0x3F).astype(np.uint16)
    b = (v & 0x1F).astype(np.uint16)
    # расширение до 8 бит тем же способом, что и в аппаратном декодере
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return np.stack([r, g, b], axis=-1).astype(np.int32)


def encode_colour_blocks(blocks):
    """blocks: (n, 16, 3) uint8 -> (n, 8) uint8, 4-цветный режим DXT1."""
    n = blocks.shape[0]

    hi = blocks.max(axis=1)
    lo = blocks.min(axis=1)

    c0 = pack565(hi)
    c1 = pack565(lo)

    # 4-цветный режим требует c0 > c1; при равенстве блок одноцветный и
    # любой индекс даёт верный результат, но c0 > c1 держим принудительно.
    swap = c0 < c1
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    equal = c0 == c1
    c1 = np.where(equal & (c1 > 0), c1 - 1, c1)

    p0 = unpack565(c0)
    p1 = unpack565(c1)
    p2 = (2 * p0 + p1) // 3
    p3 = (p0 + 2 * p1) // 3

    palette = np.stack([p0, p1, p2, p3], axis=1)          # (n, 4, 3)

    px = blocks.astype(np.int32)[:, :, None, :]            # (n, 16, 1, 3)
    diff = px - palette[:, None, :, :]                     # (n, 16, 4, 3)
    idx = (diff * diff).sum(axis=3).argmin(axis=2)         # (n, 16)

    # 16 индексов по 2 бита -> 4 байта, младший пиксель в младших битах
    idx = idx.astype(np.uint32)
    shifts = (np.arange(16, dtype=np.uint32) * 2)
    packed = (idx << shifts[None, :]).sum(axis=1, dtype=np.uint64).astype(np.uint32)

    out = np.zeros((n, 8), dtype=np.uint8)
    out[:, 0] = c0 & 0xFF
    out[:, 1] = (c0 >> 8) & 0xFF
    out[:, 2] = c1 & 0xFF
    out[:, 3] = (c1 >> 8) & 0xFF
    out[:, 4] = packed & 0xFF
    out[:, 5] = (packed >> 8) & 0xFF
    out[:, 6] = (packed >> 16) & 0xFF
    out[:, 7] = (packed >> 24) & 0xFF

    return out


def encode_alpha_blocks(alpha):
    """alpha: (n, 16) uint8 -> (n, 8) uint8, 8-градаций режим DXT5."""
    n = alpha.shape[0]

    a0 = alpha.max(axis=1).astype(np.int32)
    a1 = alpha.min(axis=1).astype(np.int32)

    # a0 > a1 -> 8 интерполированных значений
    same = a0 == a1
    a1 = np.where(same & (a1 > 0), a1 - 1, a1)
    a0 = np.where(same & (a1 == a0), a0, a0)

    steps = np.arange(8, dtype=np.int32)
    # порядок значений режима a0 > a1: a0, a1, затем 6 интерполяций
    lut = np.zeros((n, 8), dtype=np.int32)
    lut[:, 0] = a0
    lut[:, 1] = a1
    for i in range(1, 7):
        lut[:, i + 1] = ((7 - i) * a0 + i * a1) // 7

    d = np.abs(alpha.astype(np.int32)[:, :, None] - lut[:, None, :])
    idx = d.argmin(axis=2).astype(np.uint64)

    shifts = (np.arange(16, dtype=np.uint64) * 3)
    packed = (idx << shifts[None, :]).sum(axis=1, dtype=np.uint64)

    out = np.zeros((n, 8), dtype=np.uint8)
    out[:, 0] = a0.astype(np.uint8)
    out[:, 1] = a1.astype(np.uint8)
    for i in range(6):
        out[:, 2 + i] = (packed >> np.uint64(8 * i)) & 0xFF

    return out


def encode(image, dxt5):
    rgba = np.asarray(image.convert("RGBA"), dtype=np.uint8)
    rgb_blocks = to_blocks(rgba[:, :, :3])

    colour = encode_colour_blocks(rgb_blocks)

    if not dxt5:
        return colour.tobytes(), PAA_DXT1

    alpha_blocks = to_blocks(rgba[:, :, 3:4])[:, :, 0]
    alpha = encode_alpha_blocks(alpha_blocks)

    return np.concatenate([alpha, colour], axis=1).tobytes(), PAA_DXT5


def write_paa(path, data, paa_type, width, height):
    with open(path, "wb") as fh:
        fh.write(struct.pack("<H", paa_type))
        fh.write(struct.pack("<H", 0))              # тэгов/палитры нет
        fh.write(struct.pack("<HH", width, height))
        fh.write(struct.pack("<I", len(data))[:3])  # uint24 little-endian
        fh.write(data)
        fh.write(struct.pack("<H", 0))              # конец списка мипов


def parse_size(text):
    w, _, h = text.lower().partition("x")
    return int(w), int(h)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source")
    ap.add_argument("output")
    ap.add_argument("--size", help="целевой размер, напр. 2048x1024")
    ap.add_argument("--dxt5", action="store_true", help="сохранить альфа-канал")
    args = ap.parse_args()

    image = Image.open(args.source)

    if args.size:
        width, height = parse_size(args.size)
        image = image.convert("RGBA").resize((width, height), Image.LANCZOS)

    width, height = image.size

    if width % 4 or height % 4:
        sys.exit("размер должен быть кратен 4, получено %dx%d" % (width, height))

    data, paa_type = encode(image, args.dxt5)
    write_paa(args.output, data, paa_type, width, height)

    print("%s -> %s  %dx%d  %s  %d байт"
          % (args.source, args.output, width, height,
             "DXT5" if args.dxt5 else "DXT1", len(data) + 11))


if __name__ == "__main__":
    main()
