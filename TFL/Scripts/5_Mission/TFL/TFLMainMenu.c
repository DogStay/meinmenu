// 01 MAIN MENU (id 5a) + 02 MAIN MENU HOVER (id 5b)
//
// Ванильное меню не переписывается, а прячется: super.Init() отрабатывает как
// обычно (вся логика профиля/персонажа жива), после чего его виджеты
// скрываются, а поверх ложится TFL/GUI/Layouts/main_menu.layout.

modded class MainMenu
{
    protected Widget            m_TFLRoot;
    protected ImageWidget       m_TFLBg;
    protected Widget            m_TFLActionBar;
    protected Widget            m_TFLStatusBar;
    protected Widget            m_TFLLogoTitle;
    protected Widget            m_TFLLogoMark;
    protected Widget            m_TFLLogoSub;
    protected PanelWidget       m_TFLDim;

    protected ref TFL_Tooltip   m_TFLTooltip;
    protected ref array<ref TFL_MenuButton> m_TFLButtons;

    protected ref TFL_MenuButton m_BtnPlay;
    protected ref TFL_MenuButton m_BtnServers;
    protected ref TFL_MenuButton m_BtnCharacter;
    protected ref TFL_MenuButton m_BtnSettings;
    protected ref TFL_MenuButton m_BtnDiscord;
    protected ref TFL_MenuButton m_BtnWebsite;
    protected ref TFL_MenuButton m_BtnExit;

    protected ref TFL_MenuConfig m_TFLConfig;

    protected float m_TFLTime;      //!< общее время с открытия меню (сек)
    protected float m_TFLBgTime;    //!< фаза ping-pong зума фона

    override Widget Init()
    {
        Widget root = super.Init();

        if (!root)
            return root;

        m_TFLConfig  = TFL_MenuConfig.Get();
        m_TFLButtons = new array<ref TFL_MenuButton>;

        TFLHideVanilla(root);

        m_TFLRoot = GetGame().GetWorkspace().CreateWidgets("TFL/GUI/Layouts/main_menu.layout", root);

        if (!m_TFLRoot)
        {
            // Layout не найден — показываем ванильное меню обратно, а не чёрный экран.
            ErrorEx("[TFL] main_menu.layout не загрузился, откат на ванильное меню");
            TFLShowVanilla(root);
            return root;
        }

        TFLBindWidgets();
        TFLApplyConfig();

        m_TFLTime   = 0;
        m_TFLBgTime = 0;

        return root;
    }

    // ------------------------------------------------------------- binding

    protected void TFLBindWidgets()
    {
        m_TFLBg        = ImageWidget.Cast(m_TFLRoot.FindAnyWidget("tfl_bg"));
        m_TFLActionBar = m_TFLRoot.FindAnyWidget("tfl_action_bar");
        m_TFLStatusBar = m_TFLRoot.FindAnyWidget("tfl_status_bar");
        m_TFLLogoTitle = m_TFLRoot.FindAnyWidget("tfl_logo_title");
        m_TFLLogoMark  = m_TFLRoot.FindAnyWidget("tfl_logo_mark");
        m_TFLLogoSub   = m_TFLRoot.FindAnyWidget("tfl_logo_sub");
        m_TFLDim       = PanelWidget.Cast(m_TFLRoot.FindAnyWidget("tfl_dim"));

        m_TFLTooltip = new TFL_Tooltip(
            m_TFLRoot.FindAnyWidget("tfl_tooltip"),
            m_TFLRoot.FindAnyWidget("tfl_tooltip_arrow"));

        m_BtnPlay      = TFLMakeButton("tfl_btn_play",      TFL_BtnStyle.PRIMARY);
        m_BtnServers   = TFLMakeButton("tfl_btn_servers",   TFL_BtnStyle.SECONDARY);
        m_BtnCharacter = TFLMakeButton("tfl_btn_character", TFL_BtnStyle.SECONDARY);
        m_BtnSettings  = TFLMakeButton("tfl_btn_settings",  TFL_BtnStyle.SECONDARY);
        m_BtnDiscord   = TFLMakeButton("tfl_btn_discord",   TFL_BtnStyle.SECONDARY);
        m_BtnWebsite   = TFLMakeButton("tfl_btn_website",   TFL_BtnStyle.SECONDARY);
        m_BtnExit      = TFLMakeButton("tfl_btn_exit",      TFL_BtnStyle.DANGER);
    }

    protected TFL_MenuButton TFLMakeButton(string name, TFL_BtnStyle style)
    {
        ButtonWidget w = ButtonWidget.Cast(m_TFLRoot.FindAnyWidget(name));

        if (!w)
        {
            ErrorEx(string.Format("[TFL] кнопка %1 не найдена в main_menu.layout", name));
            return null;
        }

        TFL_MenuButton btn = new TFL_MenuButton(w, style);
        m_TFLButtons.Insert(btn);

        return btn;
    }

    protected void TFLApplyConfig()
    {
        TFLSetText("tfl_status_version", "VERSION " + m_TFLConfig.VersionLabel);
        TFLSetText("tfl_status_server",  "SERVER  " + m_TFLConfig.ServerName);
        TFLSetText("tfl_status_line",    m_TFLConfig.StatusLine);

        // ONLINE / PING заполняются, когда придут данные браузера серверов.
        TFLSetText("tfl_status_online",  "ONLINE  --/--");
        TFLSetText("tfl_status_ping",    "PING    -- ms");
    }

    protected void TFLSetText(string name, string value)
    {
        TextWidget w = TextWidget.Cast(m_TFLRoot.FindAnyWidget(name));

        if (w)
            w.SetText(value);
    }

    //! Прячем ванильную разметку, не трогая её логику.
    protected void TFLHideVanilla(Widget root)
    {
        Widget child = root.GetChildren();

        while (child)
        {
            if (child != m_TFLRoot)
                child.Show(false);

            child = child.GetSibling();
        }
    }

    protected void TFLShowVanilla(Widget root)
    {
        Widget child = root.GetChildren();

        while (child)
        {
            child.Show(true);
            child = child.GetSibling();
        }
    }

    // --------------------------------------------------------------- input

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!m_TFLRoot || !w)
            return super.OnClick(w, x, y, button);

        if (TFLIs(w, m_BtnPlay))      { TFLPlay();      return true; }
        if (TFLIs(w, m_BtnServers))   { TFLOpen(MENU_SERVER_BROWSER); return true; }
        if (TFLIs(w, m_BtnCharacter)) { TFLOpen(MENU_CHARACTER);      return true; }
        if (TFLIs(w, m_BtnSettings))  { TFLOpen(MENU_OPTIONS);        return true; }
        if (TFLIs(w, m_BtnDiscord))   { TFLOpenURL(m_TFLConfig.DiscordURL); return true; }
        if (TFLIs(w, m_BtnWebsite))   { TFLOpenURL(m_TFLConfig.WebsiteURL); return true; }
        if (TFLIs(w, m_BtnExit))      { GetGame().RequestExit(IDC_MAIN_QUIT); return true; }

        return super.OnClick(w, x, y, button);
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        TFL_MenuButton btn = TFLFind(w);

        if (btn)
        {
            btn.OnMouseEnter();

            // Tooltip в layout закреплён над «СЕРВЕРЫ» (id 5b), поэтому
            // показываем его только для этой кнопки.
            if (btn == m_BtnServers)
                m_TFLTooltip.Show("СПИСОК СЕРВЕРОВ", "Официальные серверы и сообщества");

            return true;
        }

        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        TFL_MenuButton btn = TFLFind(w);

        if (btn)
        {
            btn.OnMouseLeave();
            m_TFLTooltip.Hide();
            return true;
        }

        return super.OnMouseLeave(w, enterW, x, y);
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        TFL_MenuButton btn = TFLFind(w);

        if (btn)
        {
            btn.OnMouseDown();
            return true;
        }

        return super.OnMouseButtonDown(w, x, y, button);
    }

    override bool OnMouseButtonUp(Widget w, int x, int y, int button)
    {
        TFL_MenuButton btn = TFLFind(w);

        if (btn)
        {
            btn.OnMouseUp();
            return true;
        }

        return super.OnMouseButtonUp(w, x, y, button);
    }

    protected bool TFLIs(Widget w, TFL_MenuButton btn)
    {
        return btn && btn.IsEnabled() && btn.GetWidget() == w;
    }

    protected TFL_MenuButton TFLFind(Widget w)
    {
        if (!w || !m_TFLButtons)
            return null;

        foreach (TFL_MenuButton btn : m_TFLButtons)
        {
            if (btn.GetWidget() == w)
                return btn;
        }

        return null;
    }

    // ------------------------------------------------------------- actions

    protected void TFLPlay()
    {
        if (!m_TFLConfig.HasDirectConnect())
        {
            // Сервер не настроен — ведём себя как ваниль.
            TFLOpen(MENU_SERVER_BROWSER);
            return;
        }

        // ВНИМАНИЕ: имя метода прямого коннекта менялось между билдами DayZ.
        // Если компилятор ругается — сверьте с scripts/5_Mission/gui/ServerBrowser.
        g_Game.ConnectFromServerBrowser(m_TFLConfig.ServerIP, m_TFLConfig.ServerPort, m_TFLConfig.ServerPassword);
    }

    protected void TFLOpen(int menuId)
    {
        UIManager ui = GetGame().GetUIManager();

        if (ui)
            ui.EnterScriptedMenu(menuId, this);
    }

    protected void TFLOpenURL(string url)
    {
        if (url == "" || url == "https://")
            return;

        GetGame().OpenURL(url);
    }

    // -------------------------------------------------------------- update

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (!m_TFLRoot)
            return;

        m_TFLTime += timeslice;

        TFLUpdateBackground(timeslice);
        TFLUpdateIntro();

        foreach (TFL_MenuButton btn : m_TFLButtons)
            btn.Update(timeslice);

        m_TFLTooltip.Update(timeslice);

        // 5b: пока висит tooltip — фон дополнительно затемняется на 8%
        if (m_TFLDim)
            m_TFLDim.SetColor(TFL_Theme.WithAlpha(TFL_Theme.BG_0, 0.08 * TFLTooltipAlpha()));
    }

    protected float TFLTooltipAlpha()
    {
        if (m_TFLTooltip && m_TFLTooltip.IsVisible())
            return 1.0;

        return 0.0;
    }

    //! Cinematic zoom 8% за 40 s, ping-pong, linear.
    protected void TFLUpdateBackground(float timeslice)
    {
        if (!m_TFLBg)
            return;

        m_TFLBgTime += timeslice;

        float period = TFL_Theme.BG_ZOOM_TIME * 2.0;
        float phase  = m_TFLBgTime - Math.Floor(m_TFLBgTime / period) * period;
        float t      = phase / TFL_Theme.BG_ZOOM_TIME;

        if (t > 1.0)
            t = 2.0 - t;

        float scale = 1.0 + TFL_Theme.BG_ZOOM * t;
        float off   = (scale - 1.0) * 0.5;

        m_TFLBg.SetSize(scale, scale);
        m_TFLBg.SetPos(-off, -off);
    }

    //! Появление меню: логотип 0–240, лента 80–340, статус-бар 140–380 (мс).
    protected void TFLUpdateIntro()
    {
        if (m_TFLTime > 0.400)
            return;

        float ms = m_TFLTime * 1000.0;

        float aLogo   = Math.Clamp((ms -   0.0) / 240.0, 0, 1);
        float aBar    = Math.Clamp((ms -  80.0) / 260.0, 0, 1);
        float aStatus = Math.Clamp((ms - 140.0) / 240.0, 0, 1);

        if (m_TFLLogoTitle) m_TFLLogoTitle.SetAlpha(aLogo);
        if (m_TFLLogoMark)  m_TFLLogoMark.SetAlpha(aLogo);
        if (m_TFLLogoSub)   m_TFLLogoSub.SetAlpha(aLogo);
        if (m_TFLActionBar) m_TFLActionBar.SetAlpha(aBar);
        if (m_TFLStatusBar) m_TFLStatusBar.SetAlpha(aStatus);
    }
}
