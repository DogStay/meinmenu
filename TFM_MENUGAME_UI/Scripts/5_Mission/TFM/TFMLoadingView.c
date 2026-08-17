// 03 LOADING SCREEN (id 6a) и 04 центрированный вариант (id 8b).
// Самодостаточный вид: создаёт свой layout в workspace, тикает сам.
// Хост (см. TFMIntegration.c) только создаёт/удаляет его и шлёт прогресс.

class TFM_LoadingView
{
    static const string L_DEFAULT  = "TFM_MENUGAME_UI/GUI/Layouts/loading_screen.layout";
    static const string L_CENTERED = "TFM_MENUGAME_UI/GUI/Layouts/loading_centered.layout";

    protected Widget                m_Root;
    protected ref TFM_HintCard      m_Hint;
    protected ref TFM_ProgressBar   m_Progress;
    protected TextWidget            m_Caption;
    protected bool                  m_Centered;

    //! Центрированная раскладка (id 8b) для 3440×1440 / 4K включается
    //! флагом CenteredLoading в menu_config.json — надёжнее, чем угадывать
    //! разрешение из скрипта.
    void TFM_LoadingView()
    {
        m_Centered = TFM_MenuConfig.Get().CenteredLoading;

        string layout = L_DEFAULT;

        if (m_Centered)
            layout = L_CENTERED;

        m_Root = GetGame().GetWorkspace().CreateWidgets(layout);

        if (!m_Root)
        {
            ErrorEx("[TFM] loading layout не загрузился: " + layout);
            return;
        }

        m_Progress = new TFM_ProgressBar(m_Root);
        m_Caption  = TextWidget.Cast(m_Root.FindAnyWidget("tfm_load_caption"));

        Widget holder = m_Root.FindAnyWidget("tfm_load_hint_holder");

        if (holder)
        {
            string hintLayout = TFM_HintCard.L_FULL;

            if (m_Centered)
                hintLayout = TFM_HintCard.L_WIDE;

            m_Hint = new TFM_HintCard(holder, hintLayout, true);
        }
    }

    void ~TFM_LoadingView()
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

    void Show(bool show)
    {
        if (m_Root)
            m_Root.Show(show);
    }

    void SetProgress(float progress)
    {
        if (m_Progress)
            m_Progress.SetProgress(progress);
    }

    void SetCaption(string caption)
    {
        if (m_Caption)
            m_Caption.SetText(caption);
    }

    void Update(float timeslice)
    {
        if (m_Progress) m_Progress.Update(timeslice);
        if (m_Hint)     m_Hint.Update(timeslice);
    }

    bool OnClick(Widget w)
    {
        return m_Hint && m_Hint.OnClick(w);
    }
}
