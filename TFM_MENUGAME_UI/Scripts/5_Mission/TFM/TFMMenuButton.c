// Кнопка ленты действий: 5 состояний из раздела «Состояния кнопок».
// Габариты не меняются ни в одном состоянии — подменяются только цвета
// (в продакшене здесь встают StatePAA-текстуры, см. README).

enum TFM_BtnState
{
    NORMAL,
    HOVER,
    PRESSED,
    DISABLED,
    ACTIVE
}

enum TFM_BtnStyle
{
    SECONDARY,  //!< СЕРВЕРЫ / ПЕРСОНАЖ / НАСТРОЙКИ / иконки
    PRIMARY,    //!< ИГРАТЬ — заливка olive 12–20%, граница #7C8563
    DANGER      //!< ВЫХОД — граница и марка красные, без заливки
}

class TFM_MenuButton
{
    protected ButtonWidget  m_Root;
    protected PanelWidget   m_Border;
    protected PanelWidget   m_Fill;
    protected PanelWidget   m_Mark;
    protected TextWidget    m_Label;
    protected ImageWidget   m_Icon;     //!< кнопки-иконки (DISCORD / WEBSITE)

    protected TFM_BtnStyle  m_Style;
    protected TFM_BtnState  m_State;

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

    void TFM_MenuButton(ButtonWidget root, TFM_BtnStyle style)
    {
        m_Root   = root;
        m_Style  = style;
        m_State  = TFM_BtnState.NORMAL;

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

        ApplyImmediate(TFM_BtnState.NORMAL);
    }

    ButtonWidget GetWidget()
    {
        return m_Root;
    }

    TFM_BtnState GetState()
    {
        return m_State;
    }

    bool IsEnabled()
    {
        return m_State != TFM_BtnState.DISABLED;
    }

    void SetEnabled(bool enabled)
    {
        if (enabled && m_State == TFM_BtnState.DISABLED)
            SetState(TFM_BtnState.NORMAL, 0);
        else if (!enabled)
            SetState(TFM_BtnState.DISABLED, 0);
    }

    // ---------------------------------------------------------------- input

    void OnMouseEnter()
    {
        if (m_State == TFM_BtnState.DISABLED || m_State == TFM_BtnState.ACTIVE)
            return;

        SetState(TFM_BtnState.HOVER, TFM_Theme.T_TO_HOVER);
    }

    void OnMouseLeave()
    {
        if (m_State == TFM_BtnState.DISABLED || m_State == TFM_BtnState.ACTIVE)
            return;

        SetState(TFM_BtnState.NORMAL, TFM_Theme.T_TO_NORMAL);
    }

    void OnMouseDown()
    {
        if (m_State == TFM_BtnState.DISABLED)
            return;

        SetState(TFM_BtnState.PRESSED, TFM_Theme.T_TO_PRESSED);
    }

    void OnMouseUp()
    {
        if (m_State == TFM_BtnState.DISABLED)
            return;

        SetState(TFM_BtnState.HOVER, 0.100);
    }

    // ---------------------------------------------------------------- state

    void SetState(TFM_BtnState state, float duration)
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

        ApplyPressShift(state == TFM_BtnState.PRESSED);
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

        m_CurBorder = TFM_Theme.Lerp(m_ColBorderFrom, m_ColBorderTo, t);
        m_CurFill   = TFM_Theme.Lerp(m_ColFillFrom,   m_ColFillTo,   t);
        m_CurMark   = TFM_Theme.Lerp(m_ColMarkFrom,   m_ColMarkTo,   t);
        m_CurText   = TFM_Theme.Lerp(m_ColTextFrom,   m_ColTextTo,   t);

        if (m_Border) m_Border.SetColor(m_CurBorder);
        if (m_Fill)   m_Fill.SetColor(m_CurFill);
        if (m_Mark)   m_Mark.SetColor(m_CurMark);
        if (m_Label)  m_Label.SetColor(m_CurText);
        if (m_Icon)   m_Icon.SetColor(m_CurText);   //!< иконка тонируется как текст
    }

    protected void ApplyImmediate(TFM_BtnState state)
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

    protected void ResolveTarget(TFM_BtnState state)
    {
        int borderNormal = TFM_Theme.BORDER;
        int borderHover  = TFM_Theme.BORDER_HI;
        int markColor    = TFM_Theme.OLIVE;
        int fillNormal   = TFM_Theme.BTN_NORMAL;
        int fillHover    = TFM_Theme.BTN_HOVER;
        int fillPressed  = TFM_Theme.BTN_PRESSED;

        if (m_Style == TFM_BtnStyle.PRIMARY)
        {
            borderNormal = TFM_Theme.OLIVE;
            borderHover  = TFM_Theme.OLIVE_HOVER;
            fillNormal   = TFM_Theme.PRIMARY_FILL;
            fillHover    = TFM_Theme.PRIMARY_FILL_HOVER;
            fillPressed  = TFM_Theme.PRIMARY_FILL_PRESS;
        }
        else if (m_Style == TFM_BtnStyle.DANGER)
        {
            borderNormal = TFM_Theme.BORDER_DANGER;
            borderHover  = TFM_Theme.BORDER_DANGER_HI;
            markColor    = TFM_Theme.RED;
        }

        switch (state)
        {
            case TFM_BtnState.HOVER:
                m_ColBorderTo = borderHover;
                m_ColFillTo   = fillHover;
                m_ColMarkTo   = markColor;
                m_ColTextTo   = TFM_Theme.TEXT;
                break;

            case TFM_BtnState.PRESSED:
                m_ColBorderTo = borderHover;
                m_ColFillTo   = fillPressed;
                m_ColMarkTo   = TFM_Theme.OLIVE_PRESSED;
                m_ColTextTo   = 0xffc2c4be;
                break;

            case TFM_BtnState.DISABLED:
                m_ColBorderTo = TFM_Theme.WithAlpha(borderNormal, 0.40);
                m_ColFillTo   = TFM_Theme.BTN_DISABLED;
                m_ColMarkTo   = TFM_Theme.TRANSPARENT;
                m_ColTextTo   = TFM_Theme.TEXT_DISABLED;
                break;

            case TFM_BtnState.ACTIVE:
                // как NORMAL + марка olive, без подсветки фона
                m_ColBorderTo = borderNormal;
                m_ColFillTo   = fillNormal;
                m_ColMarkTo   = TFM_Theme.OLIVE;
                m_ColTextTo   = TFM_Theme.TEXT_DIM;
                break;

            default: // NORMAL
                m_ColBorderTo = borderNormal;
                m_ColFillTo   = fillNormal;
                m_ColMarkTo   = TFM_Theme.TRANSPARENT;
                m_ColTextTo   = TFM_Theme.TEXT_DIM;

                if (m_Style == TFM_BtnStyle.PRIMARY)
                {
                    m_ColMarkTo = TFM_Theme.OLIVE;   // у primary марка есть всегда
                    m_ColTextTo = TFM_Theme.TEXT;
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
