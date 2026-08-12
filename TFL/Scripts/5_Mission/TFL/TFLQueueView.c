// 05 SERVER QUEUE (id 6b).
// Индикатор ожидания: три квадрата 8×8, поочерёдная пульсация 1.2 s.

class TFL_QueueView
{
    static const string LAYOUT      = "TFL/GUI/Layouts/server_queue.layout";
    static const float  DOT_PERIOD  = 1.2;
    static const int    DOT_COUNT   = 3;

    protected Widget            m_Root;
    protected ref TFL_HintCard  m_Hint;
    protected ref TFL_MenuButton m_Cancel;

    protected TextWidget        m_Position;
    protected TextWidget        m_Total;
    protected ref array<Widget> m_Dots;

    protected float m_DotTime;

    void TFL_QueueView()
    {
        m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT);

        if (!m_Root)
        {
            ErrorEx("[TFL] server_queue.layout не загрузился");
            return;
        }

        m_Position = TextWidget.Cast(m_Root.FindAnyWidget("tfl_queue_position"));
        m_Total    = TextWidget.Cast(m_Root.FindAnyWidget("tfl_queue_total"));

        ButtonWidget cancel = ButtonWidget.Cast(m_Root.FindAnyWidget("tfl_queue_cancel"));

        if (cancel)
            m_Cancel = new TFL_MenuButton(cancel, TFL_BtnStyle.DANGER);

        m_Dots = new array<Widget>;

        for (int i = 0; i < DOT_COUNT; i++)
        {
            Widget dot = m_Root.FindAnyWidget(string.Format("tfl_queue_dot_%1", i));

            if (dot)
                m_Dots.Insert(dot);
        }

        // 05: справа тот же компонент подсказки, но без стрелок
        Widget holder = m_Root.FindAnyWidget("tfl_queue_hint_holder");

        if (holder)
            m_Hint = new TFL_HintCard(holder, TFL_HintCard.L_FULL, false);
    }

    void ~TFL_QueueView()
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

    Widget GetCancelWidget()
    {
        if (m_Cancel)
            return m_Cancel.GetWidget();

        return null;
    }

    void SetPosition(int position, int total)
    {
        if (m_Position)
            m_Position.SetText(position.ToString());

        if (m_Total)
        {
            if (total > 0)
                m_Total.SetText("/ " + total.ToString());
            else
                m_Total.SetText("");
        }
    }

    void Update(float timeslice)
    {
        if (!m_Root)
            return;

        if (m_Cancel) m_Cancel.Update(timeslice);
        if (m_Hint)   m_Hint.Update(timeslice);

        UpdateDots(timeslice);
    }

    bool OnClick(Widget w)
    {
        return m_Hint && m_Hint.OnClick(w);
    }

    bool OnMouseEnter(Widget w)
    {
        if (m_Cancel && m_Cancel.GetWidget() == w)
        {
            m_Cancel.OnMouseEnter();
            return true;
        }

        return false;
    }

    bool OnMouseLeave(Widget w)
    {
        if (m_Cancel && m_Cancel.GetWidget() == w)
        {
            m_Cancel.OnMouseLeave();
            return true;
        }

        return false;
    }

    protected void UpdateDots(float timeslice)
    {
        if (!m_Dots || m_Dots.Count() == 0)
            return;

        m_DotTime += timeslice;

        if (m_DotTime >= DOT_PERIOD)
            m_DotTime -= DOT_PERIOD;

        float slot = DOT_PERIOD / m_Dots.Count();

        for (int i = 0; i < m_Dots.Count(); i++)
        {
            float start = slot * i;
            float t     = (m_DotTime - start) / slot;
            float alpha = 0.25;

            if (t >= 0 && t <= 1)
                alpha = 0.25 + 0.75 * Math.Sin(t * Math.PI);

            m_Dots.Get(i).SetColor(TFL_Theme.WithAlpha(TFL_Theme.OLIVE, alpha));
        }
    }
}
