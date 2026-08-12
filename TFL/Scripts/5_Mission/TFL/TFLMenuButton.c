// Кнопка ленты действий: 5 состояний из раздела «Состояния кнопок».
// Габариты не меняются ни в одном состоянии — подменяются только цвета
// (в продакшене здесь встают StatePAA-текстуры, см. README).

enum TFL_BtnState
{
    NORMAL,
    HOVER,
    PRESSED,
    DISABLED,
    ACTIVE
}

enum TFL_BtnStyle
{
    SECONDARY,  //!< СЕРВЕРЫ / ПЕРСОНАЖ / НАСТРОЙКИ / иконки
    PRIMARY,    //!< ИГРАТЬ — заливка olive 12–20%, граница #7C8563
    DANGER      //!< ВЫХОД — граница и марка красные, без заливки
}

class TFL_MenuButton
{
    protected ButtonWidget  m_Root;
    protected PanelWidget   m_Border;
    protected PanelWidget   m_Fill;
    protected PanelWidget   m_Mark;
    protected TextWidget    m_Label;
    protected ImageWidget   m_Icon;     //!< кнопки-иконки (DISCORD / WEBSITE)

    protected TFL_BtnStyle  m_Style;
    protected TFL_BtnState  m_State;

    // Текущее и целевое значение анимации (0 = NORMAL, 1 = целевое состояние)
    protected int   m_ColBorderFrom, m_ColBorderTo;
    protected int   m_ColFillFrom,   m_ColFillTo;
    protected int   m_ColMarkFrom,   m_ColMarkTo;
    protected int   m_ColTextFrom,   m_ColTextTo;
    protected int   m_CurBorder, m_CurFill, m_CurMark, m_CurText;
    protected float m_Progress;
    protected float m_Duration;

    protected float m_LabelBaseY;
    protected bool  m_LabelShifted;

    void TFL_MenuButton(ButtonWidget root, TFL_BtnStyle style)
    {
        m_Root   = root;
        m_Style  = style;
        m_State  = TFL_BtnState.NORMAL;

        if (!m_Root)
            return;

        m_Border = PanelWidget.Cast(m_Root.FindAnyWidget("border"));
        m_Fill   = PanelWidget.Cast(m_Root.FindAnyWidget("fill"));
        m_Mark   = PanelWidget.Cast(m_Root.FindAnyWidget("mark"));
        m_Label  = TextWidget.Cast(m_Root.FindAnyWidget("label"));
        m_Icon   = ImageWidget.Cast(m_Root.FindAnyWidget("icon"));

        if (m_Label)
        {
            float lx, ly;
            m_Label.GetPos(lx, ly);
            m_LabelBaseY = ly;
        }

        ApplyImmediate(TFL_BtnState.NORMAL);
    }

    ButtonWidget GetWidget()
    {
        return m_Root;
    }

    TFL_BtnState GetState()
    {
        return m_State;
    }

    bool IsEnabled()
    {
        return m_State != TFL_BtnState.DISABLED;
    }

    void SetEnabled(bool enabled)
    {
        if (enabled && m_State == TFL_BtnState.DISABLED)
            SetState(TFL_BtnState.NORMAL, 0);
        else if (!enabled)
            SetState(TFL_BtnState.DISABLED, 0);
    }

    // ---------------------------------------------------------------- input

    void OnMouseEnter()
    {
        if (m_State == TFL_BtnState.DISABLED || m_State == TFL_BtnState.ACTIVE)
            return;

        SetState(TFL_BtnState.HOVER, TFL_Theme.T_TO_HOVER);
    }

    void OnMouseLeave()
    {
        if (m_State == TFL_BtnState.DISABLED || m_State == TFL_BtnState.ACTIVE)
            return;

        SetState(TFL_BtnState.NORMAL, TFL_Theme.T_TO_NORMAL);
    }

    void OnMouseDown()
    {
        if (m_State == TFL_BtnState.DISABLED)
            return;

        SetState(TFL_BtnState.PRESSED, TFL_Theme.T_TO_PRESSED);
    }

    void OnMouseUp()
    {
        if (m_State == TFL_BtnState.DISABLED)
            return;

        SetState(TFL_BtnState.HOVER, 0.100);
    }

    // ---------------------------------------------------------------- state

    void SetState(TFL_BtnState state, float duration)
    {
        if (m_State == state)
            return;

        m_State = state;

        // стартуем анимацию с того, что реально отрисовано сейчас
        m_ColBorderFrom = m_CurBorder;
        m_ColFillFrom   = m_CurFill;
        m_ColMarkFrom   = m_CurMark;
        m_ColTextFrom   = m_CurText;

        ResolveTarget(state);

        m_Duration = duration;
        m_Progress = 0;

        if (duration <= 0)
            Update(0);

        ApplyPressShift(state == TFL_BtnState.PRESSED);
    }

    void Update(float timeslice)
    {
        if (m_Duration > 0 && m_Progress < 1)
        {
            m_Progress = Math.Clamp(m_Progress + timeslice / m_Duration, 0, 1);
        }
        else
        {
            m_Progress = 1;
        }

        float t = m_Progress;

        m_CurBorder = TFL_Theme.Lerp(m_ColBorderFrom, m_ColBorderTo, t);
        m_CurFill   = TFL_Theme.Lerp(m_ColFillFrom,   m_ColFillTo,   t);
        m_CurMark   = TFL_Theme.Lerp(m_ColMarkFrom,   m_ColMarkTo,   t);
        m_CurText   = TFL_Theme.Lerp(m_ColTextFrom,   m_ColTextTo,   t);

        if (m_Border) m_Border.SetColor(m_CurBorder);
        if (m_Fill)   m_Fill.SetColor(m_CurFill);
        if (m_Mark)   m_Mark.SetColor(m_CurMark);
        if (m_Label)  m_Label.SetColor(m_CurText);
        if (m_Icon)   m_Icon.SetColor(m_CurText);   //!< иконка тонируется как текст
    }

    protected void ApplyImmediate(TFL_BtnState state)
    {
        m_State = state;
        ResolveTarget(state);

        m_ColBorderFrom = m_ColBorderTo;
        m_ColFillFrom   = m_ColFillTo;
        m_ColMarkFrom   = m_ColMarkTo;
        m_ColTextFrom   = m_ColTextTo;

        m_Progress = 1;
        m_Duration = 0;

        Update(0);
    }

    protected void ResolveTarget(TFL_BtnState state)
    {
        int borderNormal = TFL_Theme.BORDER;
        int borderHover  = TFL_Theme.BORDER_HI;
        int markColor    = TFL_Theme.OLIVE;
        int fillNormal   = TFL_Theme.BTN_NORMAL;
        int fillHover    = TFL_Theme.BTN_HOVER;
        int fillPressed  = TFL_Theme.BTN_PRESSED;

        if (m_Style == TFL_BtnStyle.PRIMARY)
        {
            borderNormal = TFL_Theme.OLIVE;
            borderHover  = TFL_Theme.OLIVE_HOVER;
            fillNormal   = TFL_Theme.PRIMARY_FILL;
            fillHover    = TFL_Theme.PRIMARY_FILL_HOVER;
            fillPressed  = TFL_Theme.PRIMARY_FILL_PRESS;
        }
        else if (m_Style == TFL_BtnStyle.DANGER)
        {
            borderNormal = TFL_Theme.BORDER_DANGER;
            borderHover  = TFL_Theme.BORDER_DANGER_HI;
            markColor    = TFL_Theme.RED;
        }

        switch (state)
        {
            case TFL_BtnState.HOVER:
                m_ColBorderTo = borderHover;
                m_ColFillTo   = fillHover;
                m_ColMarkTo   = markColor;
                m_ColTextTo   = TFL_Theme.TEXT;
                break;

            case TFL_BtnState.PRESSED:
                m_ColBorderTo = borderHover;
                m_ColFillTo   = fillPressed;
                m_ColMarkTo   = TFL_Theme.OLIVE_PRESSED;
                m_ColTextTo   = 0xffc2c4be;
                break;

            case TFL_BtnState.DISABLED:
                m_ColBorderTo = TFL_Theme.WithAlpha(borderNormal, 0.40);
                m_ColFillTo   = TFL_Theme.BTN_DISABLED;
                m_ColMarkTo   = TFL_Theme.TRANSPARENT;
                m_ColTextTo   = TFL_Theme.TEXT_DISABLED;
                break;

            case TFL_BtnState.ACTIVE:
                // как NORMAL + марка olive, без подсветки фона
                m_ColBorderTo = borderNormal;
                m_ColFillTo   = fillNormal;
                m_ColMarkTo   = TFL_Theme.OLIVE;
                m_ColTextTo   = TFL_Theme.TEXT_DIM;
                break;

            default: // NORMAL
                m_ColBorderTo = borderNormal;
                m_ColFillTo   = fillNormal;
                m_ColMarkTo   = TFL_Theme.TRANSPARENT;
                m_ColTextTo   = TFL_Theme.TEXT_DIM;

                if (m_Style == TFL_BtnStyle.PRIMARY)
                {
                    m_ColMarkTo = TFL_Theme.OLIVE;   // у primary марка есть всегда
                    m_ColTextTo = TFL_Theme.TEXT;
                }
                break;
        }
    }

    //! PRESSED — сдвиг содержимого на +2px, габариты кнопки не меняются.
    protected void ApplyPressShift(bool pressed)
    {
        if (!m_Label || pressed == m_LabelShifted)
            return;

        float w, h;
        m_Root.GetScreenSize(w, h);

        if (h <= 0)
            return;

        float lx, ly;
        m_Label.GetPos(lx, ly);

        if (pressed)
            m_Label.SetPos(lx, m_LabelBaseY + 2.0 / h);
        else
            m_Label.SetPos(lx, m_LabelBaseY);

        m_LabelShifted = pressed;
    }
}
