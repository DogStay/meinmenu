#!/usr/bin/env python3
"""Генерация UI-текстур мода: заливка, логотипы, иконки.

Логотипы и иконки — текстуры по ассет-листу хендоффа (карточка 8d:
«6 фонов · 3 логотипа · 11 панелей · 4 прогресса · 18 кнопок · 4 стрелки ·
6 иконок»). Правило «текст не запекается в текстуру» относится к подписям
кнопок (состояния, локализация), а не к логотипу — он в списке ассетов.

Шрифт: Big Shoulders — узкий гротеск, подстановка вместо Barlow Condensed
из прототипа (в игре всё равно нужен свой файл шрифта).

Фоны конвертируются отдельно: tools/png_to_paa.py.
"""

import importlib.util
import os

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, "TFL", "GUI", "textures")

FONTS = "/mnt/skills/examples/canvas-design/canvas-fonts"
FONT_CONDENSED = os.path.join(FONTS, "BigShoulders-Bold.ttf")

spec = importlib.util.spec_from_file_location("paa", os.path.join(HERE, "png_to_paa.py"))
paa = importlib.util.module_from_spec(spec)
spec.loader.exec_module(paa)

TEXT   = (236, 237, 233, 255)   # #ECEDE9
OLIVE  = (124, 133, 99, 255)    # #7C8563
SECOND = (150, 155, 150, 255)   # #969B96


def save(image, name, dxt5=True):
    path = os.path.join(OUT, name)
    data, kind = paa.encode(image, dxt5)
    paa.write_paa(path, data, kind, image.size[0], image.size[1])
    print("%-28s %sx%s  %s" % (name, image.size[0], image.size[1],
                               "DXT5" if dxt5 else "DXT1"))


def tracked_text(draw, xy, text, font, fill, tracking):
    """Отрисовка с межбуквенным интервалом (em-tracking из хендоффа)."""
    x, y = xy

    for ch in text:
        draw.text((x, y), ch, font=font, fill=fill)
        x += draw.textlength(ch, font=font) + tracking

    return x - tracking - xy[0]


def text_width(draw, text, font, tracking):
    total = sum(draw.textlength(ch, font=font) for ch in text)
    return total + tracking * (len(text) - 1)


def logo(width, height, title_px, rule_w, rule_h, sub_px, gap_a, gap_b, subtitle=True):
    """Логотип: TITLE 0.20em, марка olive, SUBTITLE 0.42em."""
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Трекинг 0.20em ощутимо расширяет строку — кегль подгоняется так,
    # чтобы строка вместе с трекингом влезла в холст с полями.
    limit = width * 0.94

    while title_px > 8:
        f_title = ImageFont.truetype(FONT_CONDENSED, title_px)
        w_title = text_width(d, "THE FIRST LINE", f_title, title_px * 0.20)

        if w_title <= limit:
            break

        title_px -= 2

    f_sub = ImageFont.truetype(FONT_CONDENSED, max(sub_px, 1))

    tr_title = title_px * 0.20
    tr_sub   = sub_px * 0.42

    # реальная высота глифов, а не кегль: у Big Shoulders большой внутренний зазор
    box = f_title.getbbox("THE FIRST LINE")
    title_h = box[3] - box[1]

    block = title_h + gap_a + rule_h
    if subtitle:
        block += gap_b + sub_px

    y = (height - block) / 2

    tracked_text(d, ((width - w_title) / 2, y - box[1]), "THE FIRST LINE", f_title, TEXT, tr_title)
    y += title_h + gap_a

    d.rectangle([(width - rule_w) / 2, y, (width + rule_w) / 2, y + rule_h], fill=OLIVE)
    y += rule_h + gap_b

    if subtitle:
        w_sub = text_width(d, "MILITARY ROLEPLAY", f_sub, tr_sub)
        tracked_text(d, ((width - w_sub) / 2, y), "MILITARY ROLEPLAY", f_sub, SECOND, tr_sub)

    return img


def icon_chat(size):
    """Иконка сообщества: скруглённое облако реплики с тремя точками."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    m = size * 0.14
    body = [m, m, size - m, size - m * 2.2]
    r = size * 0.16

    d.rounded_rectangle(body, radius=r, outline=TEXT, width=max(2, int(size * 0.055)))

    # хвостик
    tail = size * 0.13
    d.polygon([(size * 0.30, size - m * 2.2 - 1),
               (size * 0.30 + tail, size - m * 2.2 - 1),
               (size * 0.30, size - m * 0.9)], fill=TEXT)

    dot = size * 0.055
    cy = (body[1] + body[3]) / 2
    for k in (0.32, 0.5, 0.68):
        cx = size * k
        d.ellipse([cx - dot, cy - dot, cx + dot, cy + dot], fill=TEXT)

    return img


def icon_globe(size):
    """Иконка сайта: глобус — окружность, экватор и два меридиана."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    m = size * 0.13
    w = max(2, int(size * 0.055))
    box = [m, m, size - m, size - m]

    d.ellipse(box, outline=TEXT, width=w)
    d.line([m, size / 2, size - m, size / 2], fill=TEXT, width=w)

    inner = size * 0.19
    d.ellipse([size / 2 - inner, m, size / 2 + inner, size - m], outline=TEXT, width=w)
    d.line([size / 2, m, size / 2, size - m], fill=TEXT, width=w)

    return img


def vignette(size=512, strength=0.62):
    """Радиальная виньетка из хендоффа: прозрачный центр, тёмные края.
    Растягивается на весь экран, поэтому по факту эллиптическая."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    px = img.load()
    c = (size - 1) / 2.0

    for y in range(size):
        for x in range(size):
            dx, dy = (x - c) / c, (y - c) / c
            r = min(1.0, (dx * dx + dy * dy) ** 0.5 / 1.41421356)
            a = int(255 * strength * (r ** 2.2))
            px[x, y] = (9, 11, 12, a)

    return img


def main():
    os.makedirs(OUT, exist_ok=True)

    # Сплошная белая заливка: панели, границы, марки, градиенты, затемнения.
    # Цвет каждому виджету задаётся в .layout через color (RGBA).
    save(Image.new("RGBA", (32, 32), (255, 255, 255, 255)), "ui_fill.paa")

    # logo_main — виджет 700×175 (блок 700×160 из хендоффа + запас).
    # Масштаб текстуры к макету: 1024 / 700 = 1.463.
    save(logo(1024, 256, title_px=140, rule_w=140, rule_h=6, sub_px=32,
              gap_a=18, gap_b=16), "logo_main.paa")

    # logo_small 320×80 — загрузка и очередь, без подзаголовка.
    save(logo(512, 128, title_px=64, rule_w=64, rule_h=4, sub_px=0,
              gap_a=10, gap_b=0, subtitle=False), "logo_small.paa")

    save(vignette(), "vignette.paa")

    save(icon_chat(64), "icon_discord.paa")
    save(icon_globe(64), "icon_website.paa")


if __name__ == "__main__":
    main()
