#!/usr/bin/env python3
"""Оффлайн-рендер .layout в PNG — проверка геометрии без запуска игры.

Это диагностика, а не движок: воспроизводятся дерево виджетов,
относительные координаты, цвета RGBA, ui_fill-заливки и .paa-текстуры.
Шрифт подставной, поэтому кегль и метрики текста — приблизительные.

    python3 tools/preview_layout.py TFL/GUI/Layouts/main_menu.layout out.png
"""

import os
import re
import struct
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
W, H = 1920, 1080

FONTS = "/mnt/skills/examples/canvas-design/canvas-fonts"
FONT_FILE = "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf"
if not os.path.exists(FONT_FILE):
    FONT_FILE = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"


# --------------------------------------------------------------- .layout ---

class Node:
    def __init__(self, cls, name):
        self.cls, self.name = cls, name
        self.props, self.children = {}, []


def parse(path):
    text = open(path, encoding="utf-8").read()
    text = re.sub(r"//[^\n]*", "", text)
    tokens = re.findall(r'"[^"]*"|[{}]|[^\s{}]+', text)

    pos = 0

    def block(node):
        nonlocal pos
        while pos < len(tokens):
            t = tokens[pos]

            if t == "}":
                pos += 1
                return

            if t == "{":            # безымянный блок детей
                pos += 1
                block(node)
                continue

            # Class Name {   |   key value...
            if pos + 1 < len(tokens) and tokens[pos].endswith("WidgetClass"):
                child = Node(tokens[pos], tokens[pos + 1])
                pos += 2
                if pos < len(tokens) and tokens[pos] == "{":
                    pos += 1
                    block(child)
                node.children.append(child)
                continue

            # Значения свойства — либо цепочка чисел (position/size/color),
            # либо ровно один токен (строка в кавычках, blend, center).
            key = t.strip('"')
            pos += 1
            values = []

            def numeric(tok):
                try:
                    float(tok)
                    return True
                except ValueError:
                    return False

            if pos < len(tokens) and numeric(tokens[pos]):
                while pos < len(tokens) and numeric(tokens[pos]):
                    values.append(tokens[pos])
                    pos += 1
            elif pos < len(tokens) and tokens[pos] not in "{}":
                values.append(tokens[pos].strip('"'))
                pos += 1

            node.props[key] = values

    root = Node("root", "root")
    block(root)
    return root.children[0] if root.children else root


# ------------------------------------------------------------------ .paa ---

def unpack565(v):
    r, g, b = (v >> 11) & 0x1F, (v >> 5) & 0x3F, v & 0x1F
    return np.array([(r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)])


def load_paa(path):
    data = open(path, "rb").read()
    kind = struct.unpack("<H", data[:2])[0]
    w, h = struct.unpack("<HH", data[4:8])
    length = int.from_bytes(data[8:11], "little")
    blocks = data[11:11 + length]

    stride = 16 if kind == 0xFF05 else 8
    n = (w // 4) * (h // 4)
    arr = np.frombuffer(blocks, dtype=np.uint8)[:n * stride].reshape(n, stride)
    colour = arr[:, 8:] if kind == 0xFF05 else arr

    alpha = None
    if kind == 0xFF05:
        alpha = np.zeros((n, 16), dtype=np.uint8)
        for i in range(n):
            a0, a1 = int(arr[i, 0]), int(arr[i, 1])
            lut = [a0, a1] + [((7 - k) * a0 + k * a1) // 7 for k in range(1, 7)]
            bits = int.from_bytes(bytes(arr[i, 2:8]), "little")
            for k in range(16):
                alpha[i, k] = lut[(bits >> (3 * k)) & 7]

    c0 = colour[:, 0].astype(np.uint16) | (colour[:, 1].astype(np.uint16) << 8)
    c1 = colour[:, 2].astype(np.uint16) | (colour[:, 3].astype(np.uint16) << 8)
    idx = (colour[:, 4].astype(np.uint32) | (colour[:, 5].astype(np.uint32) << 8)
           | (colour[:, 6].astype(np.uint32) << 16) | (colour[:, 7].astype(np.uint32) << 24))

    out = np.zeros((n, 16, 3), dtype=np.uint8)
    for i in range(n):
        p0, p1 = unpack565(c0[i]), unpack565(c1[i])
        pal = [p0, p1, (2 * p0 + p1) // 3, (p0 + 2 * p1) // 3]
        for k in range(16):
            out[i, k] = pal[(int(idx[i]) >> (2 * k)) & 3]

    out = out.reshape(h // 4, w // 4, 4, 4, 3).transpose(0, 2, 1, 3, 4).reshape(h, w, 3)
    img = Image.fromarray(out, "RGB").convert("RGBA")

    if alpha is not None:
        a = alpha.reshape(h // 4, w // 4, 4, 4).transpose(0, 2, 1, 3).reshape(h, w)
        img.putalpha(Image.fromarray(a, "L"))

    return img


PAA_CACHE = {}


def paa(path):
    if path not in PAA_CACHE:
        full = os.path.join(ROOT, path)
        PAA_CACHE[path] = load_paa(full) if os.path.exists(full) else None
    return PAA_CACHE[path]


# ---------------------------------------------------------------- render ---

def num(node, key, default):
    v = node.props.get(key)
    return [float(x) for x in v] if v else default


def colour(node, key):
    v = node.props.get(key)
    if not v or len(v) < 4:
        return None
    r, g, b, a = [float(x) for x in v[:4]]
    return (int(r * 255), int(g * 255), int(b * 255), int(a * 255))


def draw(node, canvas, box, depth=0):
    x, y, w, h = box

    px, py = num(node, "position", [0, 0])
    pw, ph = num(node, "size", [1, 1])

    cx, cy = x + px * w, y + py * h
    cw, ch = pw * w, ph * h

    col = colour(node, "color")
    image = node.props.get("image0")

    if image and col and cw >= 1 and ch >= 1:
        size = (max(1, int(round(cw))), max(1, int(round(ch))))

        if _is_flat(image[0]):
            # белая заливка целиком заменяется цветом виджета
            tile = Image.new("RGBA", size, col)
        else:
            src = paa(image[0])
            if src is None:
                tile = None
            else:
                tile = src.resize(size, Image.LANCZOS)
                # color работает как тонировка (умножение)
                mul = np.asarray(tile, dtype=np.float32)
                mul[..., 0] *= col[0] / 255.0
                mul[..., 1] *= col[1] / 255.0
                mul[..., 2] *= col[2] / 255.0
                mul[..., 3] *= col[3] / 255.0
                tile = Image.fromarray(mul.astype(np.uint8), "RGBA")

        if tile is not None:
            canvas.alpha_composite(tile, (int(round(cx)), int(round(cy))))

    text = node.props.get("text")
    if text and text[0]:
        tc = colour(node, "text color") or (255, 255, 255, 255)
        size = max(10, int(ch * 0.62))
        font = ImageFont.truetype(FONT_FILE, size)
        d = ImageDraw.Draw(canvas)
        halign = (node.props.get("text halign") or ["left"])[0]
        tw = d.textlength(text[0], font=font)
        tx = cx + (cw - tw) / 2 if halign == "center" else (
            cx + cw - tw if halign == "right" else cx)
        d.text((tx, cy + (ch - size) / 2 - size * 0.12), text[0], font=font, fill=tc)

    for child in node.children:
        draw(child, canvas, (cx, cy, cw, ch), depth + 1)


def _is_flat(path):
    return path.endswith("ui_fill.paa")


def main():
    src, out = sys.argv[1], sys.argv[2]
    tree = parse(src)

    canvas = Image.new("RGBA", (W, H), (9, 11, 12, 255))
    draw(tree, canvas, (0, 0, W, H))
    canvas.convert("RGB").save(out)
    print("%s -> %s" % (src, out))


if __name__ == "__main__":
    main()
