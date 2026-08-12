// 08 HINT CARD (id 8a) — контроллер компонента.
// Работает с любым из трёх layout'ов (полный / компактный / широкий):
// имена виджетов в них совпадают.
//
// Автосмена каждые 8 s, ручное нажатие сбрасывает таймер.
// Смена: fade out 120 ms -> swap -> fade in 160 ms.

class TFL_HintCard
{
    static const string L_FULL    = "TFL/GUI/Layouts/hint_card.layout";
    static const string L_COMPACT = "TFL/GUI/Layouts/hint_card_compact.layout";
    static const string L_WIDE    = "TFL/GUI/Layouts/hint_card_wide.layout";

    static const float AUTO_PERIOD = 8.0;
    static const float FADE_OUT    = 0.120;
    static const float FADE_IN     = 0.160;

    protected Widget                m_Root;
    protected ImageWidget           m_Image;
    protected Widget                m_ImageDim;
    protected TextWidget            m_Category;
    protected TextWidget            m_Title;
    protected MultilineTextWidget   m_Desc;
    protected TextWidget            m_Counter;

    protected ref TFL_MenuButton    m_Prev;
    protected ref TFL_MenuButton    m_Next;

    protected ref array<ref TFL_Hint> m_Hints;
    protected int   m_Index;
    protected float m_AutoTimer;
    protected float m_Fade;         //!< 1 = видно, 0 = скрыто
    protected int   m_Pending;      //!< индекс, который встанет после fade out
    protected bool  m_Swapping;

    //! parent — контейнер нужного размера; layout — один из L_*.
    void TFL_HintCard(Widget parent, string layout, bool showArrows = true)
    {
        m_Hints = TFL_Hints.Get();
        m_Index = 0;
        m_Fade  = 1;

        m_Root = GetGame().GetWorkspace().CreateWidgets(layout, parent);

        if (!m_Root)
        {
            ErrorEx("[TFL] hint card layout не загрузился: " + layout);
            return;
        }

        m_Image    = ImageWidget.Cast(m_Root.FindAnyWidget("tfl_hint_image"));
        m_ImageDim = m_Root.FindAnyWidget("tfl_hint_image_dim");
        m_Category = TextWidget.Cast(m_Root.FindAnyWidget("tfl_hint_category"));
        m_Title    = TextWidget.Cast(m_Root.FindAnyWidget("tfl_hint_title"));
        m_Desc     = MultilineTextWidget.Cast(m_Root.FindAnyWidget("tfl_hint_desc"));
        m_Counter  = TextWidget.Cast(m_Root.FindAnyWidget("tfl_hint_counter"));

        ButtonWidget prev = ButtonWidget.Cast(m_Root.FindAnyWidget("tfl_hint_prev"));
        ButtonWidget next = ButtonWidget.Cast(m_Root.FindAnyWidget("tfl_hint_next"));

        if (showArrows)
        {
            if (prev) m_Prev = new TFL_MenuButton(prev, TFL_BtnStyle.SECONDARY);
            if (next) m_Next = new TFL_MenuButton(next, TFL_BtnStyle.SECONDARY);
        }
        else
        {
            // 05 QUEUE: тот же компонент, но без стрелок
            if (prev) prev.Show(false);
            if (next) next.Show(false);
        }

        Apply(m_Index);
    }

    Widget GetRoot()
    {
        return m_Root;
    }

    void Show(bool show)
    {
        if (m_Root)
            m_Root.Show(show);
    }

    // ---------------------------------------------------------------- input

    //! true — клик обработан карточкой.
    bool OnClick(Widget w)
    {
        if (m_Prev && m_Prev.GetWidget() == w) { Step(-1); return true; }
        if (m_Next && m_Next.GetWidget() == w) { Step( 1); return true; }

        return false;
    }

    bool OnMouseEnter(Widget w)
    {
        if (m_Prev && m_Prev.GetWidget() == w) { m_Prev.OnMouseEnter(); return true; }
        if (m_Next && m_Next.GetWidget() == w) { m_Next.OnMouseEnter(); return true; }

        return false;
    }

    bool OnMouseLeave(Widget w)
    {
        if (m_Prev && m_Prev.GetWidget() == w) { m_Prev.OnMouseLeave(); return true; }
        if (m_Next && m_Next.GetWidget() == w) { m_Next.OnMouseLeave(); return true; }

        return false;
    }

    void Step(int delta)
    {
        int count = m_Hints.Count();

        if (count == 0)
            return;

        int next = (m_Index + delta + count) % count;

        StartSwap(next);
        m_AutoTimer = 0;    //!< ручное нажатие сбрасывает таймер автосмены
    }

    // --------------------------------------------------------------- update

    void Update(float timeslice)
    {
        if (!m_Root)
            return;

        if (m_Prev) m_Prev.Update(timeslice);
        if (m_Next) m_Next.Update(timeslice);

        if (m_Swapping)
        {
            UpdateSwap(timeslice);
            return;
        }

        m_AutoTimer += timeslice;

        if (m_AutoTimer >= AUTO_PERIOD)
        {
            m_AutoTimer = 0;
            Step(1);
        }
    }

    protected void StartSwap(int newIndex)
    {
        if (m_Swapping || newIndex == m_Index)
            return;

        m_Pending  = newIndex;
        m_Swapping = true;
    }

    protected void UpdateSwap(float timeslice)
    {
        if (m_Pending != m_Index)
        {
            // fade out 120 ms
            m_Fade -= timeslice / FADE_OUT;

            if (m_Fade <= 0)
            {
                m_Fade = 0;
                Apply(m_Pending);       // swap
            }
        }
        else
        {
            // fade in 160 ms
            m_Fade += timeslice / FADE_IN;

            if (m_Fade >= 1)
            {
                m_Fade     = 1;
                m_Swapping = false;
            }
        }

        ApplyFade(m_Fade);
    }

    protected void ApplyFade(float alpha)
    {
        if (m_Image)    m_Image.SetAlpha(alpha);
        if (m_ImageDim) m_ImageDim.SetAlpha(alpha);
        if (m_Category) m_Category.SetAlpha(alpha);
        if (m_Title)    m_Title.SetAlpha(alpha);
        if (m_Desc)     m_Desc.SetAlpha(alpha);
        if (m_Counter)  m_Counter.SetAlpha(alpha);
    }

    protected void Apply(int index)
    {
        if (!m_Hints || index < 0 || index >= m_Hints.Count())
            return;

        m_Index = index;

        TFL_Hint hint = m_Hints.Get(index);

        if (m_Category) m_Category.SetText(hint.Category);
        if (m_Title)    m_Title.SetText(hint.Title);
        if (m_Desc)     m_Desc.SetText(hint.Description);
        if (m_Counter)  m_Counter.SetText(string.Format("%1 / %2", index + 1, m_Hints.Count()));

        if (m_Image)
        {
            bool hasImage = hint.Image != "";

            m_Image.Show(hasImage);

            if (m_ImageDim)
                m_ImageDim.Show(hasImage);

            if (hasImage)
                m_Image.LoadImageFile(0, hint.Image);
        }
    }
}
