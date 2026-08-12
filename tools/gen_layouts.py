#!/usr/bin/env python3
"""Генерация всех .layout мода из пиксельной геометрии хендоффа.

Хендофф задаёт абсолютные пиксели на холсте 1920×1080, а DayZ позиционирует
дочерний виджет долей от РОДИТЕЛЯ. Пересчёт делается здесь один раз, а не
двести раз на глаз.

Синтаксис сверен с рабочими layout'ами JobsMod_Client (DogStay/modik4):

  * цвет в .layout — R G B A четырьмя float 0..1 (в скрипте SetColor — ARGB!);
  * hexactpos / vexactpos / hexactsize / vexactsize = 0 — относительные
    координаты; без них движок читает position/size как пиксели;
  * сплошной заливки у PanelWidget нет — цвет несёт ImageWidget с белой
    текстурой ui_fill.paa, тонируемой через color;
  * шрифты — только реально существующие ресурсы игры.

Запуск:  python3 tools/gen_layouts.py
"""

import os

W, H = 1920.0, 1080.0

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, "TFL", "GUI", "Layouts")

FILL = "TFL/GUI/textures/ui_fill.paa"

# Единственные подтверждённые шрифтовые ресурсы (из рабочего мода).
FONT_BODY = "gui/fonts/etelkatextpro22"
FONT_MICRO = "gui/fonts/metron-bold14"

# --- палитра хендоффа ------------------------------------------------------
C_BG          = "#090B0C"
C_PANEL       = "#141819"
C_PANEL_DEEP  = "#101415"
C_BORDER      = "#23282A"
C_BORDER_HI   = "#38403A"
C_DANGER      = "#4A3335"
C_OLIVE       = "#7C8563"
C_OLIVE_DARK  = "#697254"
C_CAPS        = "#A8B18C"
C_TEXT        = "#ECEDE9"
C_TEXT_DIM    = "#D6D7D2"
C_TEXT_2ND    = "#969B96"
C_TEXT_3RD    = "#5C6360"
C_TEXT_OFF    = "#5A5E5B"
C_RED         = "#8A5C5C"

# olive 16% поверх панели, посчитано заранее (см. button())
C_PRIMARY_FILL = "#222622"


def f(v):
    s = "%.6f" % v
    s = s.rstrip("0").rstrip(".")
    return s if s else "0"


def rgba(hex_colour, alpha=1.0):
    """.layout — альфа ПОСЛЕДНЯЯ. Перепутанный порядок делает тёмные панели
    невидимыми: их альфой становится синий канал."""
    h = hex_colour.lstrip("#")
    r, g, b = int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)
    return "%s %s %s %s" % (f(r / 255.0), f(g / 255.0), f(b / 255.0), f(alpha))


class Ctx:
    """Родительский бокс в абсолютных пикселях; переводит дочерний в доли."""

    def __init__(self, x, y, w, h):
        self.x, self.y, self.w, self.h = float(x), float(y), float(w), float(h)

    def rel(self, x, y, w, h):
        return ((x - self.x) / self.w, (y - self.y) / self.h, w / self.w, h / self.h)

    def sub(self, x, y, w, h):
        return Ctx(x, y, w, h)


class Doc:
    def __init__(self, header):
        self.lines = [header, ""]

    def emit(self, depth, text):
        self.lines.append(" " * depth + text)

    def write(self, name):
        path = os.path.join(OUT, name)
        with open(path, "w", encoding="utf-8") as fh:
            fh.write("\n".join(self.lines) + "\n")
        print("%-28s %d строк" % (name, len(self.lines)))


def widget(doc, depth, cls, name, ctx, box, *, color=None, image=None,
           font=None, text=None, textcolor=None, halign=None, valign="center",
           ignore=None, visible=None, children=None):
    x, y, w, h = ctx.rel(*box)

    if ignore is None:
        ignore = children is None and cls != "ButtonWidgetClass"

    doc.emit(depth, "%s %s {" % (cls, name))
    d = depth + 1

    if font:
        doc.emit(d, 'font "%s"' % font)
    if text is not None:
        doc.emit(d, 'text "%s"' % text)
    if textcolor:
        doc.emit(d, '"text color" %s' % textcolor)
    if valign and font:
        doc.emit(d, '"text valign" %s' % valign)
    if halign and font:
        doc.emit(d, '"text halign" %s' % halign)

    doc.emit(d, "position %s %s" % (f(x), f(y)))
    doc.emit(d, "size %s %s" % (f(w), f(h)))

    if color is not None:
        doc.emit(d, "color %s" % color)

    if image:
        doc.emit(d, 'image0 "%s"' % image)
        doc.emit(d, "mode blend")
        doc.emit(d, '"src alpha" 1')
        doc.emit(d, '"stretch mode" 1')

    doc.emit(d, "hexactpos 0")
    doc.emit(d, "vexactpos 0")
    doc.emit(d, "hexactsize 0")
    doc.emit(d, "vexactsize 0")

    if ignore:
        doc.emit(d, "ignorepointer 1")
    if visible is not None:
        doc.emit(d, "visible %d" % (1 if visible else 0))
    if font:
        doc.emit(d, '"exact text" 1')

    if children:
        doc.emit(d, "{")
        children(d + 1)
        doc.emit(d, "}")

    doc.emit(depth, "}")


def fill(doc, depth, name, ctx, box, colour, alpha=1.0):
    """Сплошная заливка = белая текстура + тонировка."""
    widget(doc, depth, "ImageWidgetClass", name, ctx, box,
           color=rgba(colour, alpha), image=FILL)


def label(doc, depth, name, ctx, box, text, colour, font=FONT_BODY,
          halign="center"):
    widget(doc, depth, "TextWidgetClass", name, ctx, box, text=text,
           color="1 1 1 1", textcolor=rgba(colour), font=font, halign=halign)


def container(doc, depth, name, ctx, box, children, ignore=False):
    widget(doc, depth, "PanelWidgetClass", name, ctx, box, color="0 0 0 0",
           ignore=ignore, children=children)


def button(doc, depth, name, ctx, box, text, *, style="secondary",
           icon=None, font=FONT_BODY):
    """Кнопка: границу несёт нижний слой, заливку — врезка в 1px, марку —
    полоса 4px снизу. Скрипт перекрашивает их по имени: border/fill/mark/
    label/icon."""
    x, y, w, h = box
    inner = Ctx(x, y, w, h)

    border_c = C_BORDER
    fill_c = C_PANEL
    fill_a = 1.0
    mark_c = C_OLIVE
    mark_a = 0.0
    text_c = C_TEXT_DIM

    if style == "primary":
        border_c = C_OLIVE
        # Заливка primary — НЕПРОЗРАЧНЫЙ результат наложения olive 16% на
        # панель. Полупрозрачный слой поверх olive-границы дал бы сплошную
        # оливковую кнопку: граница лежит под заливкой во всю площадь.
        fill_c = C_PRIMARY_FILL
        fill_a = 1.0
        mark_a = 1.0
        text_c = C_TEXT
    elif style == "danger":
        border_c = C_DANGER
        mark_c = C_RED
    elif style == "disabled":
        fill_c = "#121516"
        text_c = C_TEXT_OFF

    def kids(d):
        fill(doc, d, "border", inner, (x, y, w, h), border_c)
        fill(doc, d, "fill", inner, (x + 1, y + 1, w - 2, h - 2), fill_c, fill_a)
        fill(doc, d, "mark", inner, (x, y + h - 4, w, 4), mark_c, mark_a)

        if icon:
            size = 28
            widget(doc, d, "ImageWidgetClass", "icon", inner,
                   (x + (w - size) / 2, y + (h - size) / 2, size, size),
                   color=rgba(text_c), image=icon)
        else:
            label(doc, d, "label", inner, (x + 8, y, w - 16, h - 4), text, text_c,
                  font=font)

    widget(doc, depth, "ButtonWidgetClass", name, ctx, box, color="0 0 0 0",
           children=kids)


def digit_row(doc, depth, ctx, name, x, y, h, pattern, center=False):
    """Крупное табличное число из текстур-глифов.

    Игровые шрифты, существование которых подтверждено, дают 22px — цифры
    хендоффа (таймер 92, очередь 88) ими не набрать. Поэтому каждый глиф —
    отдельный ImageWidget, а скрипт подставляет им текстуры. Ширина слота
    одинаковая у всех символов: это и есть tabular-nums из спеки.

    pattern: строка вида "dd:dd" — d = слот цифры, : = разделитель.
    center: x задаёт центр строки, иначе — её левый край."""
    dw = h * 0.46
    sw = h * 0.22
    gap = h * 0.05

    total = 0
    for ch in pattern:
        total += (sw if ch != "d" else dw) + gap
    total -= gap

    if center:
        x -= total / 2

    row = Ctx(x, y, total, h)

    def kids(d):
        cx = x
        index = 0

        for ch in pattern:
            w = sw if ch != "d" else dw

            if ch == "d":
                slot = "%s_%d" % (name, index)
                glyph = "d0"
                index += 1
            else:
                slot = "%s_sep" % name
                glyph = "dcolon"

            widget(doc, d, "ImageWidgetClass", slot, row, (cx, y, w, h),
                   color=rgba(C_TEXT),
                   image="TFL/GUI/textures/digits/%s.paa" % glyph)
            cx += w + gap

    container(doc, depth, name, ctx, (x, y, total, h), kids, ignore=True)

    return total


def root(doc, children, name="Root"):
    ctx = Ctx(0, 0, W, H)
    widget(doc, 0, "PanelWidgetClass", name, ctx, (0, 0, W, H), color="0 0 0 0",
           children=lambda d: children(d, ctx))


def gradient(doc, depth, ctx, prefix, x, y, w, h, bands, colour, top=0.0,
             bottom=0.85, invert=False):
    """Градиент собирается полосами: сплошной градиентной текстуры нет."""
    step = h / bands

    for i in range(bands):
        t = (i + 1) / float(bands)

        if invert:
            t = 1.0 - i / float(bands)
        alpha = top + (bottom - top) * (t ** 1.6)
        fill(doc, depth, "%s_%d" % (prefix, i), ctx,
             (x, y + step * i, w, step + 1), colour, alpha)


# ---------------------------------------------------------------------------
# 01/02 MAIN MENU
# ---------------------------------------------------------------------------

def main_menu():
    doc = Doc("// 01 MAIN MENU (id 5a) + 02 HOVER (id 5b). Сгенерирован tools/gen_layouts.py")

    def body(d, ctx):
        widget(doc, d, "ImageWidgetClass", "tfl_bg", ctx, (0, 0, W, H),
               color="1 1 1 1", image="TFL/GUI/textures/mainmenu_background.paa")

        # радиальная виньетка (хендофф, раздел 01): поднимает контраст
        # логотипа и ленты над светлыми участками арта
        widget(doc, d, "ImageWidgetClass", "tfl_vignette", ctx, (0, 0, W, H),
               color="1 1 1 1", image="TFL/GUI/textures/vignette.paa")

        # скрим сверху: логотип ложится на закатное небо, без него
        # светлый участок арта съедает контраст надписи
        gradient(doc, d, ctx, "tfl_scrim", 0, 0, W, 460, 5, C_BG,
                 bottom=0.45, invert=True)

        gradient(doc, d, ctx, "tfl_grad", 0, 660, W, 420, 7, C_BG)

        # доп. затемнение 8% при hover — включает скрипт
        fill(doc, d, "tfl_dim", ctx, (0, 0, W, H), C_BG, 0.0)

        # логотип — текстура (ассет-лист 8d: «3 логотипа»)
        widget(doc, d, "ImageWidgetClass", "tfl_logo_title", ctx,
               (610, 172, 700, 175), color="1 1 1 1",
               image="TFL/GUI/textures/logo_main.paa")

        # tooltip 340×110 над «СЕРВЕРЫ»
        tip = Ctx(518, 718, 340, 110)

        def tip_kids(dd):
            # tooltip поднимается скриптом: в покое слои полностью прозрачны
            fill(doc, dd, "border", tip, (518, 718, 340, 110), C_BORDER, 0.0)
            fill(doc, dd, "fill", tip, (519, 719, 338, 108), C_PANEL, 0.0)
            label(doc, dd, "tfl_tooltip_title", tip, (542, 742, 292, 30), "",
                  C_TEXT, halign="left")
            label(doc, dd, "tfl_tooltip_text", tip, (542, 776, 292, 28), "",
                  C_TEXT_2ND, font=FONT_MICRO, halign="left")

        container(doc, d, "tfl_tooltip", ctx, (518, 718, 340, 110), tip_kids,
                  ignore=True)
        fill(doc, d, "tfl_tooltip_arrow", ctx, (542, 828, 12, 8), C_PANEL, 0.0)

        # лента действий
        bar = Ctx(88, 791, 1744, 176)

        def bar_kids(dd):
            fill(doc, dd, "bar_bg", bar, (88, 791, 1744, 176), C_PANEL, 0.90)
            fill(doc, dd, "bar_top", bar, (88, 791, 1744, 1), C_BORDER)

            button(doc, dd, "tfl_btn_play", bar, (124, 827, 370, 104),
                   "ИГРАТЬ", style="primary")
            button(doc, dd, "tfl_btn_servers", bar, (518, 844, 240, 70), "СЕРВЕРЫ")
            button(doc, dd, "tfl_btn_character", bar, (782, 844, 240, 70), "ПЕРСОНАЖ")
            button(doc, dd, "tfl_btn_settings", bar, (1046, 844, 240, 70), "НАСТРОЙКИ")

            fill(doc, dd, "tfl_divider", bar, (1310, 839, 1, 80), C_BORDER)

            button(doc, dd, "tfl_btn_discord", bar, (1335, 844, 96, 70), "",
                   icon="TFL/GUI/textures/icon_discord.paa")
            button(doc, dd, "tfl_btn_website", bar, (1455, 844, 96, 70), "",
                   icon="TFL/GUI/textures/icon_website.paa")
            button(doc, dd, "tfl_btn_exit", bar, (1568, 844, 228, 70),
                   "ВЫХОД", style="danger")

        container(doc, d, "tfl_action_bar", ctx, (88, 791, 1744, 176), bar_kids)

        # статус-бар
        sb = Ctx(88, 991, 1744, 68)

        def sb_kids(dd):
            label(doc, dd, "tfl_status_online", sb, (88, 1008, 220, 34),
                  "ONLINE  --/--", C_TEXT_2ND, font=FONT_MICRO, halign="left")
            label(doc, dd, "tfl_status_server", sb, (332, 1008, 260, 34),
                  "SERVER  --", C_TEXT_2ND, font=FONT_MICRO, halign="left")
            label(doc, dd, "tfl_status_version", sb, (612, 1008, 220, 34),
                  "VERSION --", C_TEXT_2ND, font=FONT_MICRO, halign="left")
            label(doc, dd, "tfl_status_ping", sb, (852, 1008, 200, 34),
                  "PING    -- ms", C_TEXT_2ND, font=FONT_MICRO, halign="left")
            label(doc, dd, "tfl_status_line", sb, (1136, 1008, 696, 34), "",
                  C_TEXT_3RD, font=FONT_MICRO, halign="right")

        container(doc, d, "tfl_status_bar", ctx, (88, 991, 1744, 68), sb_kids,
                  ignore=True)

    root(doc, body, "TFL_MainMenuRoot")
    doc.write("main_menu.layout")


# ---------------------------------------------------------------------------
# 08 HINT CARD — три варианта
# ---------------------------------------------------------------------------

def hint_card(name, box_w, box_h, *, image, wide=False, arrows=True):
    doc = Doc("// 08 HINT CARD (id 8a). Сгенерирован tools/gen_layouts.py")

    ctx = Ctx(0, 0, box_w, box_h)
    pad = 40

    def body(d, _):
        fill(doc, d, "tfl_hint_border", ctx, (0, 0, box_w, box_h), C_BORDER)
        fill(doc, d, "tfl_hint_fill", ctx, (1, 1, box_w - 2, box_h - 2), C_PANEL)

        if image:
            if wide:
                iw, ih = box_w * 0.34, box_h
                widget(doc, d, "ImageWidgetClass", "tfl_hint_image", ctx,
                       (0, 0, iw, ih), color="1 1 1 1", image=FILL)
                fill(doc, d, "tfl_hint_image_dim", ctx,
                     (0, ih * 0.7, iw, ih * 0.3), C_BG, 0.5)
                tx = iw + pad
            else:
                ih = box_w * 3.0 / 8.0     # жёстко 8:3
                widget(doc, d, "ImageWidgetClass", "tfl_hint_image", ctx,
                       (0, 0, box_w, ih), color="1 1 1 1", image=FILL)
                fill(doc, d, "tfl_hint_image_dim", ctx,
                     (0, ih * 0.7, box_w, ih * 0.3), C_BG, 0.5)
                tx = pad
        else:
            ih = 0
            tx = pad

        top = ih + pad if not wide else pad
        tw = box_w - tx - pad

        label(doc, d, "tfl_hint_category", ctx, (tx, top, tw, 26), "",
              C_OLIVE, font=FONT_MICRO, halign="left")
        label(doc, d, "tfl_hint_title", ctx, (tx, top + 40, tw, 40), "",
              C_TEXT, halign="left")
        widget(doc, d, "MultilineTextWidgetClass", "tfl_hint_desc", ctx,
               (tx, top + 96, tw, 117), text="", color="1 1 1 1",
               textcolor=rgba(C_TEXT_DIM), font=FONT_BODY, halign="left")

        label(doc, d, "tfl_hint_counter", ctx,
              (tx, box_h - pad - 24, tw * 0.5, 24), "", C_TEXT_2ND,
              font=FONT_MICRO, halign="left")

        ay = box_h - pad - 44
        ax = box_w - pad - 56
        button(doc, d, "tfl_hint_prev", ctx, (ax - 64, ay, 56, 44), "<",
               style="secondary" if arrows else "disabled")
        button(doc, d, "tfl_hint_next", ctx, (ax, ay, 56, 44), ">",
               style="secondary" if arrows else "disabled")

    widget(doc, 0, "PanelWidgetClass", "TFL_HintCardRoot", ctx,
           (0, 0, box_w, box_h), color="0 0 0 0",
           children=lambda d: body(d, ctx))
    doc.write(name)


# ---------------------------------------------------------------------------
# 03/04 LOADING
# ---------------------------------------------------------------------------

def progress(doc, d, ctx, x, y, w, h):
    track = Ctx(x, y, w, h)

    def kids(dd):
        fill(doc, dd, "border", track, (x, y, w, h), C_BORDER)
        fill(doc, dd, "tfl_progress_bg", track, (x + 1, y + 1, w - 2, h - 2),
             C_PANEL_DEEP)
        fill(doc, dd, "tfl_progress_fill", track, (x + 1, y + 1, 0, h - 2),
             C_OLIVE_DARK)
        fill(doc, dd, "tfl_progress_caps", track, (x + 1, y - 4, 4, h + 8), C_CAPS)

        for pct in (25, 50, 75):
            fill(doc, dd, "tick_%d" % pct, track,
                 (x + w * pct / 100.0, y, 1, h), C_BORDER)

    container(doc, d, "tfl_progress_track", ctx, (x, y, w, h), kids, ignore=True)


def loading_screen():
    doc = Doc("// 03 LOADING SCREEN (id 6a). Сгенерирован tools/gen_layouts.py")

    def body(d, ctx):
        widget(doc, d, "ImageWidgetClass", "tfl_load_bg", ctx, (0, 0, W, H),
               color="1 1 1 1",
               image="TFL/GUI/textures/loading_background_01.paa")

        gradient(doc, d, ctx, "tfl_load_grad", 0, 660, W, 420, 4, C_BG,
                 bottom=0.80)

        widget(doc, d, "ImageWidgetClass", "tfl_load_logo", ctx,
               (176, 120, 320, 80), color="1 1 1 1",
               image="TFL/GUI/textures/logo_small.paa")

        container(doc, d, "tfl_load_hint_holder", ctx, (1232, 180, 600, 460),
                  lambda dd: None)

        label(doc, d, "tfl_load_label", ctx, (88, 856, 400, 26),
              "ЗАГРУЗКА МИРА", C_TEXT_2ND, font=FONT_MICRO, halign="left")

        progress(doc, d, ctx, 88, 900, 1280, 28)

        label(doc, d, "tfl_progress_percent", ctx, (1392, 896, 200, 40), "0%",
              C_TEXT, halign="left")
        label(doc, d, "tfl_load_caption", ctx, (88, 952, 700, 24),
              "CHERNARUS · ЗАГРУЗКА ЛАНДШАФТА", C_TEXT_3RD, font=FONT_MICRO,
              halign="left")
        label(doc, d, "tfl_load_warning", ctx, (88, 984, 700, 24),
              "НЕ ВЫКЛЮЧАЙТЕ ИГРУ ДО ЗАВЕРШЕНИЯ ЗАГРУЗКИ", C_TEXT_3RD,
              font=FONT_MICRO, halign="left")

    root(doc, body, "TFL_LoadingRoot")
    doc.write("loading_screen.layout")


def loading_centered():
    doc = Doc("// 04 LOADING, центрированный (id 8b). Сгенерирован tools/gen_layouts.py")

    def body(d, ctx):
        widget(doc, d, "ImageWidgetClass", "tfl_load_bg", ctx, (0, 0, W, H),
               color="1 1 1 1",
               image="TFL/GUI/textures/loading_background_02.paa")

        fill(doc, d, "tfl_load_dim", ctx, (0, 0, W, H), C_BG, 0.35)

        widget(doc, d, "ImageWidgetClass", "tfl_load_logo", ctx,
               (800, 120, 320, 80), color="1 1 1 1",
               image="TFL/GUI/textures/logo_small.paa")

        container(doc, d, "tfl_load_hint_holder", ctx, (460, 400, 1000, 340),
                  lambda dd: None)

        progress(doc, d, ctx, 460, 800, 1000, 24)

        label(doc, d, "tfl_progress_percent", ctx, (460, 840, 1000, 40), "0%",
              C_TEXT)
        label(doc, d, "tfl_load_caption", ctx, (460, 900, 1000, 24),
              "НЕ ВЫКЛЮЧАЙТЕ ИГРУ ДО ЗАВЕРШЕНИЯ ЗАГРУЗКИ", C_TEXT_3RD,
              font=FONT_MICRO)

    root(doc, body, "TFL_LoadingCenteredRoot")
    doc.write("loading_centered.layout")


# ---------------------------------------------------------------------------
# 05 SERVER QUEUE
# ---------------------------------------------------------------------------

def server_queue():
    doc = Doc("// 05 SERVER QUEUE (id 6b). Сгенерирован tools/gen_layouts.py")

    PX, PY, PW, PH = 300, 330, 800, 420
    PAD = 40

    def body(d, ctx):
        widget(doc, d, "ImageWidgetClass", "tfl_queue_bg", ctx, (0, 0, W, H),
               color="1 1 1 1",
               image="TFL/GUI/textures/loading_background_03.paa")

        # blur 6px движком UI не даётся — компенсируется затемнением 65%
        fill(doc, d, "tfl_queue_dim", ctx, (0, 0, W, H), C_BG, 0.65)
        widget(doc, d, "ImageWidgetClass", "tfl_queue_vignette", ctx,
               (0, 0, W, H), color="1 1 1 1",
               image="TFL/GUI/textures/vignette.paa")

        panel = Ctx(PX, PY, PW, PH)
        left = PX + PAD

        def kids(dd):
            fill(doc, dd, "border", panel, (PX, PY, PW, PH), C_BORDER)
            fill(doc, dd, "fill", panel, (PX + 1, PY + 1, PW - 2, PH - 2), C_PANEL)
            fill(doc, dd, "mark", panel, (PX, PY, PW, 4), C_OLIVE)

            label(doc, dd, "tfl_queue_title", panel, (left, PY + 42, PW - PAD * 2, 28),
                  "ПОДКЛЮЧЕНИЕ К СЕРВЕРУ", C_TEXT, halign="left")
            label(doc, dd, "tfl_queue_label", panel, (left, PY + 80, PW - PAD * 2, 22),
                  "Ваша позиция в очереди", C_TEXT_2ND, font=FONT_MICRO,
                  halign="left")

            # 88px tabular — текстурные глифы, до трёх разрядов
            row = digit_row(doc, dd, panel, "tfl_queue_digits",
                            left, PY + 122, 88, "ddd")

            # «/ NN» — 36px вторичным, по нижней линии числа
            label(doc, dd, "tfl_queue_total", panel,
                  (left + row + 20, PY + 168, 200, 34), "", C_TEXT_2ND,
                  halign="left")

            label(doc, dd, "tfl_queue_waiting", panel, (left, PY + 246, 300, 22),
                  "Ожидание подключения", C_TEXT_2ND, font=FONT_MICRO,
                  halign="left")

            # индикатор: три квадрата 8×8, поочерёдная пульсация 1.2 s
            for i in range(3):
                fill(doc, dd, "tfl_queue_dot_%d" % i, panel,
                     (left + 250 + i * 20, PY + 253, 8, 8), C_OLIVE)

            button(doc, dd, "tfl_queue_cancel", panel,
                   (left, PY + 296, 300, 70), "ОТМЕНА", style="danger")

        container(doc, d, "tfl_queue_panel", ctx, (PX, PY, PW, PH), kids)

        container(doc, d, "tfl_queue_hint_holder", ctx, (1232, 340, 600, 400),
                  lambda dd: None)

        label(doc, d, "tfl_queue_caption", ctx, (PX, 980, 900, 24),
              "СРЕДНЕЕ ВРЕМЯ ОЖИДАНИЯ ~ 4 МИН · НЕ ЗАКРЫВАЙТЕ КЛИЕНТ",
              C_TEXT_3RD, font=FONT_MICRO, halign="left")

    root(doc, body, "TFL_QueueRoot")
    doc.write("server_queue.layout")


# ---------------------------------------------------------------------------
# 06/07 TIMERS
# ---------------------------------------------------------------------------

def timer_panel():
    doc = Doc("// 06 LOGIN / 07 RESPAWN TIMER (id 6c). Сгенерирован tools/gen_layouts.py")

    # Панель 600×340; вертикальный ритм пересчитан под реальный кегль текста
    # (22px) и крупные цифры-текстуры — иначе середина панели пустует.
    PX, PY, PW, PH = 660, 370, 600, 340

    def body(d, ctx):
        fill(doc, d, "tfl_timer_dim", ctx, (0, 0, W, H), C_BG, 0.80)

        panel = Ctx(PX, PY, PW, PH)

        def kids(dd):
            fill(doc, dd, "border", panel, (PX, PY, PW, PH), C_BORDER)
            fill(doc, dd, "fill", panel, (PX + 1, PY + 1, PW - 2, PH - 2), C_PANEL)
            fill(doc, dd, "tfl_timer_mark", panel, (PX, PY, PW, 4), C_OLIVE)

            label(doc, dd, "tfl_timer_title", panel, (PX + 40, PY + 34, PW - 80, 28),
                  "ВХОД В МИР", C_TEXT)

            # 92px tabular — текстурные глифы, по центру панели
            digit_row(doc, dd, panel, "tfl_timer_digits",
                      PX + PW / 2, PY + 82, 92, "dd:dd", center=True)

            label(doc, dd, "tfl_timer_subtitle", panel,
                  (PX + 30, PY + 190, PW - 60, 24),
                  "Не отключайте игру во время подключения", C_TEXT_2ND,
                  font=FONT_MICRO)

            label(doc, dd, "tfl_timer_character", panel,
                  (PX + 30, PY + 218, PW - 60, 22), "", C_TEXT_3RD,
                  font=FONT_MICRO)

            # login: полоса обратного отсчёта 360×12
            track = Ctx(PX + (PW - 360) / 2, PY + 262, 360, 12)

            def track_kids(ddd):
                fill(doc, ddd, "bg", track, (PX + (PW - 360) / 2, PY + 262, 360, 12),
                     C_PANEL_DEEP)
                fill(doc, ddd, "tfl_timer_fill", track,
                     (PX + (PW - 360) / 2, PY + 262, 360, 12), C_OLIVE_DARK)

            container(doc, dd, "tfl_timer_track", panel,
                      (PX + (PW - 360) / 2, PY + 262, 360, 12), track_kids,
                      ignore=True)

            # respawn: кнопка 300×70 по центру, подпись под ней
            button(doc, dd, "tfl_timer_respawn", panel,
                   (PX + (PW - 300) / 2, PY + 232, 300, 70), "ВОЗРОДИТЬСЯ",
                   style="disabled")
            label(doc, dd, "tfl_timer_hint", panel,
                  (PX + 30, PY + 310, PW - 60, 20), "КНОПКА АКТИВНА ПРИ 00:00",
                  C_TEXT_3RD, font=FONT_MICRO)

        container(doc, d, "tfl_timer_panel", ctx, (PX, PY, PW, PH), kids)

    root(doc, body, "TFL_TimerRoot")
    doc.write("timer_panel.layout")


# ---------------------------------------------------------------------------
# 09/10 DIALOGS
# ---------------------------------------------------------------------------

def dialog():
    doc = Doc("// 09/10 DIALOGS (id 7a). Сгенерирован tools/gen_layouts.py")

    def body(d, ctx):
        fill(doc, d, "tfl_dialog_dim", ctx, (0, 0, W, H), C_BG, 0.70)

        panel = Ctx(600, 360, 720, 360)

        def kids(dd):
            fill(doc, dd, "border", panel, (600, 360, 720, 360), C_BORDER)
            fill(doc, dd, "fill", panel, (601, 361, 718, 358), C_PANEL)
            fill(doc, dd, "tfl_dialog_mark", panel, (600, 360, 720, 4), C_OLIVE)

            label(doc, dd, "tfl_dialog_title", panel, (640, 400, 640, 30),
                  "ПОДТВЕРЖДЕНИЕ", C_TEXT, halign="left")
            widget(doc, dd, "MultilineTextWidgetClass", "tfl_dialog_text", panel,
                   (640, 460, 640, 78), text="", color="1 1 1 1",
                   textcolor=rgba(C_TEXT_DIM), font=FONT_BODY, halign="left")
            label(doc, dd, "tfl_dialog_code", panel, (640, 550, 640, 24), "",
                  C_RED, font=FONT_MICRO, halign="left")

            fill(doc, dd, "tfl_dialog_sep", panel, (640, 586, 640, 1), C_BORDER)

            button(doc, dd, "tfl_dialog_btn_left", panel, (656, 610, 300, 70),
                   "ОТМЕНА")
            button(doc, dd, "tfl_dialog_btn_right", panel, (980, 610, 300, 70),
                   "ВЫЙТИ", style="danger")

        container(doc, d, "tfl_dialog_panel", ctx, (600, 360, 720, 360), kids)

    root(doc, body, "TFL_DialogRoot")
    doc.write("dialog.layout")


# ---------------------------------------------------------------------------
# 8c SECTION PANEL
# ---------------------------------------------------------------------------

def section_panel():
    doc = Doc("// Панель раздела (id 8c). Сгенерирован tools/gen_layouts.py")

    px, py, pw, ph = 88, 260, 1744, 571

    def body(d, ctx):
        fill(doc, d, "tfl_section_dim", ctx, (0, 0, W, H), C_BG, 0.80)

        panel = Ctx(px, py, pw, ph)
        cols = [("name", 0.03, 0.30), ("map", 0.36, 0.14),
                ("players", 0.52, 0.12), ("mode", 0.66, 0.14),
                ("ping", 0.82, 0.10)]
        titles = ["СЕРВЕР", "КАРТА", "ИГРОКИ", "РЕЖИМ", "ПИНГ"]

        def kids(dd):
            fill(doc, dd, "border", panel, (px, py, pw, ph), C_BORDER)
            fill(doc, dd, "fill", panel, (px + 1, py + 1, pw - 2, ph - 2), C_PANEL)

            inner_x, inner_w = px + 30, pw - 60

            for (cell, cx, cw), title in zip(cols, titles):
                label(doc, dd, "hdr_" + cell, panel,
                      (inner_x + inner_w * cx, py + 20, inner_w * cw, 26),
                      title, C_TEXT_2ND, font=FONT_MICRO, halign="left")

            fill(doc, dd, "hdr_sep", panel, (inner_x, py + 60, inner_w, 1), C_BORDER)

            row_h, step, top = 64, 68, py + 70

            for i in range(6):
                ry = top + step * i
                row = Ctx(inner_x, ry, inner_w, row_h)

                def row_kids(ddd, ry=ry, row=row):
                    fill(doc, ddd, "border", row,
                         (inner_x, ry, inner_w, row_h), C_BORDER)
                    fill(doc, ddd, "fill", row,
                         (inner_x + 1, ry + 1, inner_w - 2, row_h - 2), C_PANEL)
                    fill(doc, ddd, "mark", row, (inner_x, ry, 3, row_h),
                         C_OLIVE, 0.0)

                    for cell, cx, cw in cols:
                        label(doc, ddd, cell, row,
                              (inner_x + inner_w * cx + 12, ry + 19,
                               inner_w * cw - 24, 26),
                              "", C_TEXT_DIM, font=FONT_MICRO, halign="left")

                widget(doc, dd, "ButtonWidgetClass", "tfl_row_%d" % i, panel,
                       (inner_x, ry, inner_w, row_h), color="0 0 0 0",
                       children=row_kids)

            label(doc, dd, "tfl_section_desc", panel,
                  (inner_x, py + ph - 120, inner_w * 0.6, 60), "", C_TEXT_2ND,
                  font=FONT_MICRO, halign="left")

            button(doc, dd, "tfl_section_action", panel,
                   (px + pw - 40 - 300, py + ph - 110, 300, 70),
                   "ПОДКЛЮЧИТЬСЯ", style="primary")

        container(doc, d, "tfl_section_panel", ctx, (px, py, pw, ph), kids)

    root(doc, body, "TFL_SectionRoot")
    doc.write("section_panel.layout")


def main():
    os.makedirs(OUT, exist_ok=True)

    main_menu()
    hint_card("hint_card.layout", 800, 660, image=True)
    hint_card("hint_card_compact.layout", 800, 320, image=False)
    hint_card("hint_card_wide.layout", 1000, 340, image=True, wide=True)
    loading_screen()
    loading_centered()
    server_queue()
    timer_panel()
    dialog()
    section_panel()


if __name__ == "__main__":
    main()
