// 13 PROGRESS (id 8b) — трек с заливкой, caps-риской и процентом.
// Заливка идёт lerp'ом 250 ms и никогда не откатывается назад.

class TFM_ProgressBar
{
    static const float LERP_TIME = 0.250;

    protected Widget     m_Track;
    protected Widget     m_Bg;
    protected Widget     m_Fill;
    protected Widget     m_Caps;
    protected TextWidget m_Percent;

    protected float m_InnerX;      //!< левый край заливки в координатах трека
    protected float m_InnerW;      //!< полная ширина заливки при 100%
    protected float m_InnerH;

    protected float m_Target;
    protected float m_Current;

    void TFM_ProgressBar(Widget root)
    {
        if (!root)
            return;

        m_Track   = root.FindAnyWidget("tfm_progress_track");
        m_Percent = TextWidget.Cast(root.FindAnyWidget("tfm_progress_percent"));

        if (!m_Track)
            return;

        m_Bg   = m_Track.FindAnyWidget("tfm_progress_bg");
        m_Fill = m_Track.FindAnyWidget("tfm_progress_fill");
        m_Caps = m_Track.FindAnyWidget("tfm_progress_caps");

        if (m_Bg)
        {
            float y;
            m_Bg.GetPos(m_InnerX, y);
            m_Bg.GetSize(m_InnerW, m_InnerH);
        }

        Apply(0);
    }

    //! progress — 0..1. Откат назад игнорируется (см. «lerp 250 ms, без отката»).
    void SetProgress(float progress)
    {
        progress = Math.Clamp(progress, 0, 1);

        if (progress > m_Target)
            m_Target = progress;
    }

    void Update(float timeslice)
    {
        if (!m_Fill || m_Current >= m_Target)
            return;

        float step = timeslice / LERP_TIME;

        m_Current = Math.Clamp(m_Current + step, 0, m_Target);

        Apply(m_Current);
    }

    protected void Apply(float value)
    {
        if (m_Fill)
            m_Fill.SetSize(m_InnerW * value, m_InnerH);

        if (m_Caps)
        {
            float cw, ch;
            m_Caps.GetSize(cw, ch);

            float cx, cy;
            m_Caps.GetPos(cx, cy);

            m_Caps.SetPos(m_InnerX + m_InnerW * value - cw, cy);
            m_Caps.Show(value > 0.001);
        }

        if (m_Percent)
        {
            int percent = Math.Round(value * 100);
            m_Percent.SetText(percent.ToString() + "%");
        }
    }
}
