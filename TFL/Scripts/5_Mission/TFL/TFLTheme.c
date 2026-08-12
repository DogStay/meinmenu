// Design tokens — "THE FIRST LINE | MILITARY RP" UI handoff.
// Значения берутся из раздела Design Tokens и «Состояния кнопок».
// Формат — ARGB (0xAARRGGBB), как ожидает Widget.SetColor().

class TFL_Theme
{
    // Фон
    static const int BG_0            = 0xff090b0c;
    static const int BG_1            = 0xff0d1011;
    static const int BG_2            = 0xff111415;

    // Панели
    static const int PANEL           = 0xff141819;
    static const int PANEL_HI        = 0xff181d1e;

    // Границы
    static const int BORDER          = 0xff23282a;
    static const int BORDER_HI       = 0xff38403a;
    static const int BORDER_DANGER   = 0xff4a3335;
    static const int BORDER_DANGER_HI= 0xff6a4547;

    // Акцент (olive)
    static const int OLIVE_DARK      = 0xff697254;
    static const int OLIVE           = 0xff7c8563;
    static const int OLIVE_HOVER     = 0xff96a07a;
    static const int OLIVE_PRESSED   = 0xff5e6749;
    static const int OLIVE_CAPS      = 0xffa8b18c;

    // Текст
    static const int TEXT            = 0xffecede9;
    static const int TEXT_DIM        = 0xffd6d7d2;
    static const int TEXT_2ND        = 0xff969b96;
    static const int TEXT_3RD        = 0xff5c6360;
    static const int TEXT_DISABLED   = 0xff5a5e5b;

    // Красный — только ошибки, предупреждения, выход
    static const int RED             = 0xff8a5c5c;
    static const int RED_LIGHT       = 0xffc0a2a2;

    // Состояния кнопок: фон
    static const int BTN_NORMAL      = 0xff141819;
    static const int BTN_HOVER       = 0xff1c2122;
    static const int BTN_PRESSED     = 0xff0f1213;
    static const int BTN_DISABLED    = 0xff121516;

    // PRIMARY — заливка olive 12–20%
    static const int PRIMARY_FILL       = 0x1f697254;
    static const int PRIMARY_FILL_HOVER = 0x33697254;

    static const int TRANSPARENT     = 0x00000000;

    // Тайминги переходов (сек), таблица Interactions & Behavior
    static const float T_TO_HOVER    = 0.180;
    static const float T_TO_NORMAL   = 0.120;
    static const float T_TO_PRESSED  = 0.080;
    static const float T_TOOLTIP_DELAY = 0.400;
    static const float T_TOOLTIP_FADE  = 0.120;

    // Фон: zoom 8% за 40 s, ping-pong
    static const float BG_ZOOM       = 0.08;
    static const float BG_ZOOM_TIME  = 40.0;

    //! Линейная интерполяция двух ARGB-цветов покомпонентно.
    static int Lerp(int from, int to, float t)
    {
        if (t <= 0) return from;
        if (t >= 1) return to;

        int a = Math.Lerp((from >> 24) & 0xff, (to >> 24) & 0xff, t);
        int r = Math.Lerp((from >> 16) & 0xff, (to >> 16) & 0xff, t);
        int g = Math.Lerp((from >>  8) & 0xff, (to >>  8) & 0xff, t);
        int b = Math.Lerp( from        & 0xff,  to        & 0xff, t);

        return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    }

    //! Тот же цвет с другой альфой (0..1).
    static int WithAlpha(int color, float alpha)
    {
        int a = Math.Clamp(alpha * 255, 0, 255);
        return (color & 0x00ffffff) | ((a & 0xff) << 24);
    }
}
