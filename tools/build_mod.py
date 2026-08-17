#!/usr/bin/env python3
"""Сборка готового мода: build/@TFL/ и build/TFL_mod.zip.

Шаги:
  1. стейджинг — копия TFL/ без GUI/textures/_source (исходные PNG в PBO
     не нужны, это 11 МБ мусора в раздаче);
  2. упаковка PBO (tools/pack_pbo.py);
  3. проверка — PBO распаковывается обратно и файлы сверяются с исходными,
     плюс контроль SHA1-подписи в хвосте;
  4. zip-архив для раздачи игрокам.

Запуск:  python3 tools/build_mod.py
"""

import hashlib
import importlib.util
import os
import shutil
import struct
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

SRC = os.path.join(ROOT, "TFL")
BUILD = os.path.join(ROOT, "build")
STAGE = os.path.join(BUILD, "stage", "TFL")
MODDIR = os.path.join(BUILD, "@TFL")
PBO = os.path.join(MODDIR, "Addons", "TFL.pbo")
ZIP = os.path.join(BUILD, "TFL_mod.zip")

PREFIX = "TFL"
EXCLUDE = {os.path.join("GUI", "textures", "_source")}

README = """TFL — интерфейс DayZ "THE FIRST LINE | MILITARY RP"

УСТАНОВКА
1. Положить папку @TFL рядом с DayZ_x64.exe
2. Запустить игру с параметром:  -mod=@TFL

НАСТРОЙКА СЕРВЕРА
При первом запуске создастся файл:
    <профиль DayZ>/TFL/menu_config.json

В нём указываются адрес сервера, ссылки и версия:
    "ServerIP":   "203.0.113.10"
    "ServerPort": 2302
    "DiscordURL": "https://discord.gg/..."
    "WebsiteURL": "https://..."

Кнопка ИГРАТЬ делает прямой коннект на этот адрес.
Пока ServerIP оставлен как 127.0.0.1 — открывается обычный браузер серверов.

Файл правится без пересборки PBO, достаточно перезапустить игру.

"CenteredLoading": true — центрированный экран загрузки для 3440x1440 и 4K.

ЕСЛИ ЧТО-ТО НЕ ЗАПУСКАЕТСЯ
Экраны загрузки, очереди и таймеров цепляются к ванильным классам DayZ,
имена которых меняются между версиями игры. Всё это собрано в одном файле
Scripts/5_Mission/TFL/TFLIntegration.c — при ошибках компиляции правится
или удаляется только он, главное меню от этого не ломается.
"""


def stage():
    if os.path.exists(BUILD):
        shutil.rmtree(BUILD)

    def ignore(directory, names):
        rel = os.path.relpath(directory, SRC)
        rel = "" if rel == "." else rel
        return [n for n in names if os.path.join(rel, n) in EXCLUDE]

    shutil.copytree(SRC, STAGE, ignore=ignore)

    return sum(len(files) for _, _, files in os.walk(STAGE))


def pack():
    spec = importlib.util.spec_from_file_location(
        "pack", os.path.join(HERE, "pack_pbo.py"))
    module = importlib.util.module_from_spec(spec)

    # pack_pbo.py читает sys.argv на импорте
    argv = sys.argv
    sys.argv = ["pack_pbo.py", STAGE, PBO, PREFIX]
    try:
        spec.loader.exec_module(module)
    finally:
        sys.argv = argv


def verify():
    """PBO должен распаковываться обратно бит в бит."""
    data = open(PBO, "rb").read()
    pos = 1 + 20                      # пустая запись заголовка

    def cstring():
        nonlocal pos
        out = bytearray()
        while data[pos] != 0:
            out.append(data[pos])
            pos += 1
        pos += 1
        return out.decode("utf-8", "replace")

    props = {}
    while True:
        key = cstring()
        if not key:
            break
        props[key] = cstring()

    entries = []
    while True:
        name = cstring()
        fields = struct.unpack_from("<5I", data, pos)
        pos += 20
        if not name and fields[4] == 0:
            break
        entries.append((name, fields[4]))

    offset = pos
    mismatch = 0

    for name, size in entries:
        blob = data[offset:offset + size]
        offset += size

        original = open(os.path.join(STAGE, name.replace("\\", os.sep)), "rb").read()
        if blob != original:
            mismatch += 1
            print("  РАСХОЖДЕНИЕ: %s" % name)

    tail = data[offset:]
    signed = tail[1:21] == hashlib.sha1(data[:offset]).digest()

    print("  prefix:        %s" % props.get("prefix"))
    print("  файлов в PBO:  %d" % len(entries))
    print("  совпадают:     %s" % ("все" if not mismatch else "НЕТ, %d" % mismatch))
    print("  SHA1-подпись:  %s" % ("ок" if signed else "НЕ СХОДИТСЯ"))

    return not mismatch and signed and props.get("prefix") == PREFIX


def archive():
    with open(os.path.join(MODDIR, "README.txt"), "w", encoding="utf-8") as fh:
        fh.write(README)

    with zipfile.ZipFile(ZIP, "w", zipfile.ZIP_DEFLATED) as zf:
        for base, _, files in os.walk(MODDIR):
            for name in files:
                full = os.path.join(base, name)
                zf.write(full, os.path.join("@TFL", os.path.relpath(full, MODDIR)))


def main():
    count = stage()
    print("стейджинг: %d файлов (без GUI/textures/_source)" % count)

    pack()

    print("проверка:")
    if not verify():
        sys.exit("PBO собран неправильно")

    archive()

    print("готово:")
    print("  %s" % os.path.relpath(MODDIR, ROOT))
    print("  %s (%.1f МБ)" % (os.path.relpath(ZIP, ROOT),
                              os.path.getsize(ZIP) / 1048576.0))


if __name__ == "__main__":
    main()
