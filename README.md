# TFL — интерфейс DayZ по хендоффу THE FIRST LINE | MILITARY RP

Клиентский мод: полная замена визуальной части интерфейса по
`docs/DESIGN_HANDOFF.md`. Реализованы все 14 экранов ТЗ.

Ванильные меню не переписываются: `super.Init()` отрабатывает как обычно
(логика профиля, персонажа, серверов, очереди жива), после чего ванильные
виджеты скрываются, а поверх ложится наш layout. Если layout не загрузится —
ванильный экран возвращается на место, чёрного экрана не будет.

## Экраны

| # | Экран | id | Layout | Контроллер |
|---|---|---|---|---|
| 01 | Главное меню | 5a | `main_menu.layout` | `TFLMainMenu.c` |
| 02 | Hover + tooltip | 5b | там же | `TFLTooltip.c` |
| 03 | Загрузка | 6a | `loading_screen.layout` | `TFLLoadingView.c` |
| 04 | Загрузка, центрированная | 8b | `loading_centered.layout` | там же |
| 05 | Очередь на сервер | 6b | `server_queue.layout` | `TFLQueueView.c` |
| 06 | Таймер входа | 6c | `timer_panel.layout` | `TFLTimerView.c` |
| 07 | Таймер респавна | 6c | там же (режим RESPAWN) | там же |
| 08 | Hint card | 8a | `hint_card*.layout` ×3 | `TFLHintCard.c` |
| 09/10 | Диалоги | 7a | `dialog.layout` | `TFLDialogMenu.c` |
| 11–14 | UI-kit / состояния / прогресс / ассеты | 7b, 4a, 8b, 8d | — | `TFLTheme.c`, `TFLMenuButton.c`, `TFLProgressBar.c` |
| — | Панель раздела | 8c | `section_panel.layout` | `TFLSectionPanel.c` |

Справочные листы 11–14 — это не экраны, а спецификации; они реализованы как код:
токены в `TFLTheme`, матрица состояний в `TFLMenuButton`, прогресс в `TFLProgressBar`.

## Структура

```
TFL/
  $PBOPREFIX$                     TFL
  config.cpp                      CfgPatches + CfgMods (missionScriptModule)
  GUI/Layouts/*.layout            10 файлов разметки
  GUI/textures/                   сюда кладутся .paa
  GUI/textures/_source/           исходные PNG фонов (в PBO не нужны)
  Scripts/5_Mission/TFL/
    TFLTheme.c         design tokens, тайминги, цветовой lerp
    TFLMenuConfig.c    JSON-конфиг в $profile:TFL/menu_config.json
    TFLMenuButton.c    5 состояний кнопки, 3 стиля (primary/secondary/danger)
    TFLTooltip.c       tooltip 340×110
    TFLProgressBar.c   трек, заливка, caps-риска, процент
    TFLHints.c         12 подсказок с финальным текстом
    TFLHintCard.c      компонент подсказки (полный/компактный/широкий)
    TFLMainMenu.c      главное меню + панель раздела + диалог выхода
    TFLLoadingView.c   загрузка
    TFLQueueView.c     очередь
    TFLTimerView.c     таймеры входа/респавна
    TFLSectionPanel.c  панель раздела
    TFLDialogMenu.c    диалоги
    TFLIntegration.c   ЕДИНСТВЕННЫЕ хуки в ванильные классы
tools/                 pack_pbo.py, list_pbo.py, sha256_file.py (из WorkKit)
```

## Настройка

`$profile:TFL/menu_config.json` создаётся с дефолтами при первом запуске,
правится без пересборки PBO:

```json
{
  "ServerName": "THE FIRST LINE | MILITARY RP",
  "ServerIP": "203.0.113.10",
  "ServerPort": 2302,
  "ServerPassword": "",
  "DiscordURL": "https://discord.gg/...",
  "WebsiteURL": "https://...",
  "StatusLine": "THE FIRST LINE | MILITARY RP  ·  CHERNARUS",
  "VersionLabel": "1.0.0",
  "CenteredLoading": false
}
```

`CenteredLoading: true` — центрированная раскладка загрузки (id 8b) для
3440×1440 и 4K. Флагом, а не автоопределением: угадывать разрешение из
скрипта менее надёжно, чем задать явно.

**ИГРАТЬ** делает прямой коннект на этот адрес. Пока `ServerIP` дефолтный
(`127.0.0.1`) — открывается обычный браузер серверов, чтобы меню работало
«из коробки».

## Сборка

1. **Текстуры.** PNG из `GUI/textures/_source/` → PAA через **TexView 2** или
   `ImageToPAA.exe`, положить в `GUI/textures/` под именами из layout'ов:
   `mainmenu_background.paa`, `loading_background_01.paa`,
   `loading_background_02.paa`, `loading_background_03.paa`.
   Перед конвертацией привести к степени двойки, мип-мапы выключить.
   Кастомные энкодеры не использовать.
2. **PBO.** Прекомпиляция скриптов — Workbench, затем:
   ```
   python tools/pack_pbo.py TFL @TFL/Addons/TFL.pbo TFL
   ```
3. Положить `@TFL` рядом с DayZ и запустить с `-mod=@TFL`.

`$PBOPREFIX$` = `TFL`, значит в layout — `TFL/GUI/textures/...`.
Никаких `P:/`, `C:/`, `D:/`.

## Что нужно доделать руками

Три пункта, которые нельзя закрыть из текстового репозитория:

1. **Шрифты и выравнивание текста.** В `.layout` не проставлены `font` и
   выравнивание: имена шрифтовых ресурсов отличаются между билдами DayZ, а
   угаданное имя даёт невидимый текст. Открыть layout'ы в **DayZ Layout
   Editor**, задать узкий гротеск (аналог Barlow Condensed) и моноширинный
   для микротекста, выставить размеры по типографике из хендоффа (TITLE 96,
   таймер 92, процент 40, TITLE карточки 34, BUTTON 32, BODY 26, SMALL 20) и
   центрирование подписей кнопок. Геометрия, цвета и иерархия виджетов уже
   расставлены — трогать их не нужно.
2. **PAA-ассеты.** Панели, границы и марки собраны из `PanelWidget` с
   цветами из токенов — цвета точные, но это не 9-patch. По ассет-листу
   (карточка 8d, 52 файла) они заменяются на 9-patch PAA + StatePAA; логика
   состояний в `TFLMenuButton` уже разведена по слотам
   `border` / `fill` / `mark` / `label`. Иконки DISCORD / WEBSITE — пока
   текстовые заглушки `DS` / `WWW`. Виньетка, шум 5% и blur (6px в очереди,
   8px в диалогах) требуют PAA и не реализованы: blur компенсирован
   затемнением, градиенты собраны из полос.
3. **`TFLIntegration.c` — сверить с вашим билдом.** Это единственный файл,
   который трогает ванильные классы (`LoadingScreen`, `LoginQueueBase`,
   `LoginTimeBase`, `RespawnDialogue`). Их имена и сигнатуры менялись между
   версиями DayZ. Если компилятор ругается — правится или удаляется только
   он: вьюхи самодостаточны, главное меню не сломается.
   Внутри же три места, куда нужно подставить вызовы своего билда:
   - `TFLSetProgress` — прокинуть реальный прогресс загрузки;
   - `TFLSetPosition` / `TFLSetTime` — реальные позиция в очереди и отсчёт;
   - в `RespawnDialogue.OnClick` — вызов ванильного респавна (наш
     `ButtonWidget` не тот, на который завязан оригинальный обработчик).
   Плюс `g_Game.ConnectFromServerBrowser(...)` в `TFLMainMenu.TFLPlay()` —
   сверить с `scripts/5_Mission/gui/ServerBrowser`.

## Решения, которые я принял сам

- **Вертикальная раскладка низа главного меню.** В спеке `action_bar`
  одновременно «отступ снизу 80», `status_bar` «отступ снизу 21» и «зазор до
  ленты 24» — три условия не сходятся (80 + 176 + 24 + 68 > 1080 снизу).
  Взята привязка к низу экрана: status_bar 21 от низа, зазор 24, лента выше
  → `action_bar.y = 791`.
- **Ширина вторичных кнопок 240 вместо 260.** С 260 лента переполняется на
  67px: `36 + 370 + 24 + 3×(260+24) + 1 + 24 + 2×(96+24) + 228 + 36 > 1744`.
  При 240 всё встаёт с зазором 17px до правого края.
- **Панель раздела открывается только для «СЕРВЕРЫ».** «ПЕРСОНАЖ» и
  «НАСТРОЙКИ» ведут в ванильные меню: пустая красивая панель хуже рабочих
  настроек. Компонент `TFL_SectionPanel` универсальный — переключается одной
  строкой в `TFLMainMenu.OnClick`, когда для этих разделов появится
  содержимое.
- **Список серверов в панели раздела** — сейчас одна строка из конфига.
  Точка подстановки реального списка: `TFLMainMenu.TFLBuildServerRows()`.
- **Диалоги показываются через `UIManager.ShowScriptedMenu`**, а не через
  собственный `MENU_*` id — не требует регистрации в фабрике меню и не
  конфликтует с другими модами.

## Осталось за рамками

- Два арта фона по ТЗ ещё не нарисованы (разрушенный город как отдельный
  loading, лесной военный лагерь) — в хендоффе они тоже помечены как
  недоделанные.
- Векторные иконки discord / website.
- Focus-рамка для геймпада (`focus_frame`) — в `TFLMenuButton` состояние
  заложено, отдельный виджет рамки не добавлен.
