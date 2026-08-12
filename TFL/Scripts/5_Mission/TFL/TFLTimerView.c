// 06 LOGIN TIMER / 07 RESPAWN TIMER (id 6c).
// Одна панель, два режима. Таймер 92px tabular, микропульс яркости 6%
// каждую секунду. Respawn: кнопка ВОЗРОДИТЬСЯ в DISABLED до 00:00.

enum TFL_TimerMode
{
    LOGIN,
    RESPAWN
}

class TFL_TimerView
{
    static const string LAYOUT = "TFL/GUI/Layouts/timer_panel.layout";

    protected Widget            m_Root;
    protected Widget            m_Panel;
    protected Widget            m_Mark;
    protected TextWidget        m_Title;
    protected ref TFL_Digits    m_Value;   //!< 92px — текстурные глифы
    protected TextWidget        m_Subtitle;
    protected TextWidget        m_Character;
    protected TextWidget        m_Hint;
    protected Widget            m_Track;
    protected Widget            m_Fill;
    protected ref TFL_MenuButton m_Respawn;

    protected TFL_TimerMode m_Mode;
    protected float m_Total;
    protected float m_Left;
    protected float m_PulseTime;
    protected float m_TrackW, m_TrackH;

    void TFL_TimerView(TFL_TimerMode mode)
    {
        m_Mode = mode;
        m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT);

        if (!m_Root)
        {
            ErrorEx("[TFL] timer_panel.layout не загрузился");
            return;
        }

        // Цвет панели несёт слой border внутри контейнера, а не сам контейнер:
        // контейнер прозрачный (color 0 0 0 0), заливку рисует ImageWidget.
        Widget panel = m_Root.FindAnyWidget("tfl_timer_panel");

        if (panel)
            m_Panel = panel.FindAnyWidget("border");
        m_Mark      = m_Root.FindAnyWidget("tfl_timer_mark");
        m_Title     = TextWidget.Cast(m_Root.FindAnyWidget("tfl_timer_title"));
        m_Value     = new TFL_Digits(m_Root, "tfl_timer_digits", 4);
        m_Subtitle  = TextWidget.Cast(m_Root.FindAnyWidget("tfl_timer_subtitle"));
        m_Character = TextWidget.Cast(m_Root.FindAnyWidget("tfl_timer_character"));
        m_Hint      = TextWidget.Cast(m_Root.FindAnyWidget("tfl_timer_hint"));
        m_Track     = m_Root.FindAnyWidget("tfl_timer_track");

        if (m_Track)
        {
            m_Fill = m_Track.FindAnyWidget("tfl_timer_fill");

            if (m_Fill)
                m_Fill.GetSize(m_TrackW, m_TrackH);
        }

        ButtonWidget respawn = ButtonWidget.Cast(m_Root.FindAnyWidget("tfl_timer_respawn"));

        if (respawn)
            m_Respawn = new TFL_MenuButton(respawn, TFL_BtnStyle.DANGER);

        ApplyMode();
    }

    void ~TFL_TimerView()
    {
        Destroy();
    }

    void Destroy()
    {
        if (m_Root)
        {
            m_Root.Unlink();
            m_Root = null;
        }
    }

    Widget GetRoot()
    {
        return m_Root;
    }

    Widget GetRespawnWidget()
    {
        if (m_Respawn)
            return m_Respawn.GetWidget();

        return null;
    }

    bool IsRespawnEnabled()
    {
        return m_Respawn && m_Respawn.IsEnabled();
    }

    void SetCharacter(string text)
    {
        if (m_Character)
            m_Character.SetText(text);
    }

    //! seconds — сколько осталось; total задаёт масштаб полосы отсчёта.
    void SetTime(float seconds, float total = -1)
    {
        if (total > 0)
            m_Total = total;
        else if (m_Total <= 0)
            m_Total = Math.Max(seconds, 1);

        m_Left = Math.Max(seconds, 0);

        if (m_Value)
            m_Value.SetValue(FormatTime(m_Left));

        if (m_Fill && m_Total > 0)
            m_Fill.SetSize(m_TrackW * Math.Clamp(m_Left / m_Total, 0, 1), m_TrackH);

        if (m_Mode == TFL_TimerMode.RESPAWN && m_Respawn)
        {
            bool ready = m_Left <= 0;

            m_Respawn.SetEnabled(ready);

            if (m_Hint)
                m_Hint.Show(!ready);
        }
    }

    void Update(float timeslice)
    {
        if (!m_Root)
            return;

        if (m_Respawn)
            m_Respawn.Update(timeslice);

        UpdatePulse(timeslice);
    }

    bool OnMouseEnter(Widget w)
    {
        if (m_Respawn && m_Respawn.GetWidget() == w)
        {
            m_Respawn.OnMouseEnter();
            return true;
        }

        return false;
    }

    bool OnMouseLeave(Widget w)
    {
        if (m_Respawn && m_Respawn.GetWidget() == w)
        {
            m_Respawn.OnMouseLeave();
            return true;
        }

        return false;
    }

    //! Микропульс яркости 6% с периодом 1 s.
    protected void UpdatePulse(float timeslice)
    {
        if (!m_Value)
            return;

        m_PulseTime += timeslice;

        if (m_PulseTime >= 1.0)
            m_PulseTime -= 1.0;

        float alpha = 0.94 + 0.06 * Math.Sin(m_PulseTime * Math.PI2);

        m_Value.SetAlpha(alpha);
    }

    protected void ApplyMode()
    {
        bool respawn = m_Mode == TFL_TimerMode.RESPAWN;

        if (m_Mark)
            m_Mark.SetColor(respawn ? TFL_Theme.RED : TFL_Theme.OLIVE);

        if (m_Panel)
            m_Panel.SetColor(respawn ? TFL_Theme.BORDER_DANGER : TFL_Theme.BORDER);

        if (m_Title)
            m_Title.SetText(respawn ? "ВОЗВРАЩЕНИЕ В МИР" : "ВХОД В МИР");

        if (m_Subtitle)
            m_Subtitle.SetText(respawn
                ? "Персонаж потерян. Ожидайте появления."
                : "Не отключайте игру во время подключения");

        // login: полоса отсчёта, без кнопки; respawn: наоборот
        if (m_Track)
            m_Track.Show(!respawn);

        if (m_Respawn)
        {
            m_Respawn.GetWidget().Show(respawn);
            m_Respawn.SetEnabled(false);
        }

        if (m_Hint)
            m_Hint.Show(respawn);

        if (m_Character)
            m_Character.Show(!respawn);
    }

    //! "MMSS" — четыре разряда, двоеточие рисует отдельный слот.
    protected string FormatTime(float seconds)
    {
        int total = Math.Round(seconds);
        int mm    = Math.Min(total / 60, 99);
        int ss    = total % 60;

        // без двоеточия: разделитель — отдельный слот в layout
        return Pad(mm) + Pad(ss);
    }

    protected string Pad(int value)
    {
        if (value < 10)
            return "0" + value.ToString();

        return value.ToString();
    }
}
