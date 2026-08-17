// Tooltip 340×110: задержка 400 ms, fade 120 ms (id 5b / 7a).
// Всегда внутри safe-zone — позиция задана в layout, стрелка 12×8 у левого края.

class TFM_Tooltip
{
    protected Widget     m_Root;
    protected Widget     m_Border;
    protected Widget     m_Fill;
    protected Widget     m_Arrow;
    protected TextWidget m_Title;
    protected TextWidget m_Text;

    protected bool  m_Requested;
    protected float m_Delay;
    protected float m_Alpha;

    void TFM_Tooltip(Widget root, Widget arrow)
    {
        m_Root  = root;
        m_Arrow = arrow;

        if (m_Root)
        {
            m_Border = m_Root.FindAnyWidget("border");
            m_Fill   = m_Root.FindAnyWidget("fill");
            m_Title  = TextWidget.Cast(m_Root.FindAnyWidget("tfm_tooltip_title"));
            m_Text   = TextWidget.Cast(m_Root.FindAnyWidget("tfm_tooltip_text"));
        }

        Apply(0);
    }

    //! Запросить показ. Реальное появление — через 400 ms.
    void Show(string title, string text)
    {
        if (m_Title) m_Title.SetText(title);
        if (m_Text)  m_Text.SetText(text);

        if (!m_Requested)
            m_Delay = TFM_Theme.T_TOOLTIP_DELAY;

        m_Requested = true;
    }

    void Hide()
    {
        m_Requested = false;
        m_Delay     = 0;
    }

    bool IsVisible()
    {
        return m_Alpha > 0;
    }

    void Update(float timeslice)
    {
        if (m_Requested)
        {
            if (m_Delay > 0)
            {
                m_Delay -= timeslice;
                return;
            }

            m_Alpha = Math.Clamp(m_Alpha + timeslice / TFM_Theme.T_TOOLTIP_FADE, 0, 1);
        }
        else
        {
            if (m_Alpha <= 0)
                return;

            m_Alpha = Math.Clamp(m_Alpha - timeslice / TFM_Theme.T_TOOLTIP_FADE, 0, 1);
        }

        Apply(m_Alpha);
    }

    protected void Apply(float alpha)
    {
        // Гасим слои по отдельности: у каждого своя альфа, гашение
        // контейнера на них не распространяется.
        if (m_Border) m_Border.SetColor(TFM_Theme.WithAlpha(TFM_Theme.BORDER, alpha));
        if (m_Fill)   m_Fill.SetColor(TFM_Theme.WithAlpha(TFM_Theme.PANEL, alpha));
        if (m_Title)  m_Title.SetAlpha(alpha);
        if (m_Text)   m_Text.SetAlpha(alpha);

        if (m_Arrow)
            m_Arrow.SetColor(TFM_Theme.WithAlpha(TFM_Theme.PANEL, alpha));
    }
}
