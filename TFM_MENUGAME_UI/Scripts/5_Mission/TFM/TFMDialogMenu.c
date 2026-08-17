// 09/10 DIALOGS + TOOLTIP (id 7a).
// Одна панель на три типа: ПОДТВЕРЖДЕНИЕ / ОШИБКА ПОДКЛЮЧЕНИЯ / ИНФОРМАЦИЯ.
// Esc = отмена, Enter = подтверждение.
//
// Показывается через UIManager.ShowScriptedMenu, поэтому не требует
// регистрации собственного MENU_* id.

enum TFM_DialogType
{
    CONFIRM,
    ERROR,
    INFO
}

class TFM_DialogMenu extends UIScriptedMenu
{
    static const string LAYOUT = "TFM_MENUGAME_UI/GUI/Layouts/dialog.layout";

    protected TFM_DialogType m_Type;
    protected string m_Title;
    protected string m_Text;
    protected string m_Code;
    protected string m_LabelLeft;
    protected string m_LabelRight;

    protected ref TFM_MenuButton m_BtnLeft;
    protected ref TFM_MenuButton m_BtnRight;

    //! Invoke(bool confirmed)
    ref ScriptInvoker Event_OnResult = new ScriptInvoker();

    void TFM_DialogMenu()
    {
        m_Type       = TFM_DialogType.CONFIRM;
        m_Title      = "ПОДТВЕРЖДЕНИЕ";
        m_LabelLeft  = "ОТМЕНА";
        m_LabelRight = "ВЫЙТИ";
    }

    // ------------------------------------------------------------- фабрики

    static TFM_DialogMenu ShowConfirm(string title, string text, string confirmLabel)
    {
        TFM_DialogMenu menu = new TFM_DialogMenu();
        menu.Configure(TFM_DialogType.CONFIRM, title, text, "", "ОТМЕНА", confirmLabel);

        return Present(menu);
    }

    static TFM_DialogMenu ShowError(string text, string code)
    {
        TFM_DialogMenu menu = new TFM_DialogMenu();
        menu.Configure(TFM_DialogType.ERROR, "ОШИБКА ПОДКЛЮЧЕНИЯ", text, code, "ЗАКРЫТЬ", "");

        return Present(menu);
    }

    static TFM_DialogMenu ShowInfo(string text)
    {
        TFM_DialogMenu menu = new TFM_DialogMenu();
        menu.Configure(TFM_DialogType.INFO, "ИНФОРМАЦИЯ", text, "", "", "ОК");

        return Present(menu);
    }

    protected static TFM_DialogMenu Present(TFM_DialogMenu menu)
    {
        UIManager ui = GetGame().GetUIManager();

        if (!ui)
            return null;

        ui.ShowScriptedMenu(menu, null);

        return menu;
    }

    void Configure(TFM_DialogType type, string title, string text, string code, string left, string right)
    {
        m_Type       = type;
        m_Title      = title;
        m_Text       = text;
        m_Code       = code;
        m_LabelLeft  = left;
        m_LabelRight = right;
    }

    // ---------------------------------------------------------------- init

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT);

        if (!layoutRoot)
        {
            ErrorEx("[TFM] dialog.layout не загрузился");
            return layoutRoot;
        }

        TextWidget title = TextWidget.Cast(layoutRoot.FindAnyWidget("tfm_dialog_title"));
        MultilineTextWidget text = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("tfm_dialog_text"));
        TextWidget code  = TextWidget.Cast(layoutRoot.FindAnyWidget("tfm_dialog_code"));
        Widget     mark  = layoutRoot.FindAnyWidget("tfm_dialog_mark");

        // Границу панели рисует слой border внутри прозрачного контейнера.
        Widget panel = layoutRoot.FindAnyWidget("tfm_dialog_panel");

        if (panel)
            panel = panel.FindAnyWidget("border");

        if (title) title.SetText(m_Title);
        if (text)  text.SetText(m_Text);

        if (code)
        {
            code.SetText(m_Code);
            code.Show(m_Code != "");
        }

        // ОШИБКА: граница панели #4A3335, марка #8a5c5c. Остальные — olive.
        bool isError = m_Type == TFM_DialogType.ERROR;

        if (mark)  mark.SetColor(isError ? TFM_Theme.RED : TFM_Theme.OLIVE);
        if (panel) panel.SetColor(isError ? TFM_Theme.BORDER_DANGER : TFM_Theme.BORDER);

        SetupButtons();

        return layoutRoot;
    }

    protected void SetupButtons()
    {
        ButtonWidget left  = ButtonWidget.Cast(layoutRoot.FindAnyWidget("tfm_dialog_btn_left"));
        ButtonWidget right = ButtonWidget.Cast(layoutRoot.FindAnyWidget("tfm_dialog_btn_right"));

        // secondary слева, акцентная справа; пустая подпись = кнопки нет
        if (left)
        {
            if (m_LabelLeft != "")
            {
                TextWidget label = TextWidget.Cast(left.FindAnyWidget("label"));

                if (label)
                    label.SetText(m_LabelLeft);

                m_BtnLeft = new TFM_MenuButton(left, TFM_BtnStyle.SECONDARY);
            }
            else
            {
                left.Show(false);
            }
        }

        if (right)
        {
            if (m_LabelRight != "")
            {
                TextWidget rlabel = TextWidget.Cast(right.FindAnyWidget("label"));

                if (rlabel)
                    rlabel.SetText(m_LabelRight);

                // ВЫЙТИ — danger, ОК — primary
                TFM_BtnStyle style = TFM_BtnStyle.PRIMARY;

                if (m_Type == TFM_DialogType.CONFIRM)
                    style = TFM_BtnStyle.DANGER;

                m_BtnRight = new TFM_MenuButton(right, style);
            }
            else
            {
                right.Show(false);
            }
        }
    }

    // --------------------------------------------------------------- input

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_BtnLeft && m_BtnLeft.GetWidget() == w)
        {
            Finish(false);
            return true;
        }

        if (m_BtnRight && m_BtnRight.GetWidget() == w)
        {
            // У ERROR/INFO единственная кнопка справа — это «закрыть/ОК».
            Finish(m_Type == TFM_DialogType.CONFIRM);
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (m_BtnLeft  && m_BtnLeft.GetWidget()  == w) { m_BtnLeft.OnMouseEnter();  return true; }
        if (m_BtnRight && m_BtnRight.GetWidget() == w) { m_BtnRight.OnMouseEnter(); return true; }

        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (m_BtnLeft  && m_BtnLeft.GetWidget()  == w) { m_BtnLeft.OnMouseLeave();  return true; }
        if (m_BtnRight && m_BtnRight.GetWidget() == w) { m_BtnRight.OnMouseLeave(); return true; }

        return super.OnMouseLeave(w, enterW, x, y);
    }

    override bool OnKeyPress(Widget w, int x, int y, int key)
    {
        if (key == KeyCode.KC_ESCAPE)
        {
            Finish(false);
            return true;
        }

        if (key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER)
        {
            Finish(m_Type == TFM_DialogType.CONFIRM || m_LabelRight != "");
            return true;
        }

        return super.OnKeyPress(w, x, y, key);
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (m_BtnLeft)  m_BtnLeft.Update(timeslice);
        if (m_BtnRight) m_BtnRight.Update(timeslice);
    }

    protected void Finish(bool confirmed)
    {
        Event_OnResult.Invoke(confirmed);

        UIManager ui = GetGame().GetUIManager();

        if (ui)
            ui.Back();
    }
}
