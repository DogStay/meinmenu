// 01 MAIN MENU (id 5a) + 02 MAIN MENU HOVER (id 5b)
//
// Ванильное меню не переписывается, а прячется: super.Init() отрабатывает как
// обычно (вся логика профиля/персонажа жива), после чего его виджеты
// скрываются, а поверх ложится TFM_MENUGAME_UI/GUI/Layouts/main_menu.layout.

modded class MainMenu
{
    protected Widget            m_TFMRoot;
    protected ImageWidget       m_TFMBg;
    protected Widget            m_TFMActionBar;
    protected Widget            m_TFMStatusBar;
    //! Логотип — одна текстура logo_main.paa (марка и подзаголовок внутри неё).
    protected Widget            m_TFMLogo;
    protected PanelWidget       m_TFMDim;

    protected ref TFM_Tooltip   m_TFMTooltip;
    protected ref array<ref TFM_MenuButton> m_TFMButtons;

    protected ref TFM_MenuButton m_BtnPlay;
    protected ref TFM_MenuButton m_BtnServers;
    protected ref TFM_MenuButton m_BtnCharacter;
    protected ref TFM_MenuButton m_BtnSettings;
    protected ref TFM_MenuButton m_BtnDiscord;
    protected ref TFM_MenuButton m_BtnWebsite;
    protected ref TFM_MenuButton m_BtnExit;

    protected ref TFM_MenuConfig m_TFMConfig;

    protected ref TFM_SectionPanel m_TFMSection;
    protected bool  m_TFMSectionOpen;
    protected float m_TFMBarY, m_TFMBarH;   //!< габариты ленты до сжатия

    protected float m_TFMTime;      //!< общее время с открытия меню (сек)
    protected float m_TFMBgTime;    //!< фаза ping-pong зума фона

    override Widget Init()
    {
        Widget root = super.Init();

        if (!root)
            return root;

        m_TFMConfig  = TFM_MenuConfig.Get();
        m_TFMButtons = new array<ref TFM_MenuButton>;

        TFMHideVanilla(root);

        m_TFMRoot = GetGame().GetWorkspace().CreateWidgets("TFM_MENUGAME_UI/GUI/Layouts/main_menu.layout", root);

        if (!m_TFMRoot)
        {
            // Layout не найден — показываем ванильное меню обратно, а не чёрный экран.
            ErrorEx("[TFM] main_menu.layout не загрузился, откат на ванильное меню");
            TFMShowVanilla(root);
            return root;
        }

        TFMBindWidgets();
        TFMApplyConfig();

        m_TFMTime   = 0;
        m_TFMBgTime = 0;

        return root;
    }

    // ------------------------------------------------------------- binding

    protected void TFMBindWidgets()
    {
        m_TFMBg        = ImageWidget.Cast(m_TFMRoot.FindAnyWidget("tfm_bg"));
        m_TFMActionBar = m_TFMRoot.FindAnyWidget("tfm_action_bar");
        m_TFMStatusBar = m_TFMRoot.FindAnyWidget("tfm_status_bar");
        m_TFMLogo      = m_TFMRoot.FindAnyWidget("tfm_logo_title");
        m_TFMDim       = PanelWidget.Cast(m_TFMRoot.FindAnyWidget("tfm_dim"));

        m_TFMTooltip = new TFM_Tooltip(
            m_TFMRoot.FindAnyWidget("tfm_tooltip"),
            m_TFMRoot.FindAnyWidget("tfm_tooltip_arrow"));

        m_BtnPlay      = TFMMakeButton("tfm_btn_play",      TFM_BtnStyle.PRIMARY);
        m_BtnServers   = TFMMakeButton("tfm_btn_servers",   TFM_BtnStyle.SECONDARY);
        m_BtnCharacter = TFMMakeButton("tfm_btn_character", TFM_BtnStyle.SECONDARY);
        m_BtnSettings  = TFMMakeButton("tfm_btn_settings",  TFM_BtnStyle.SECONDARY);
        m_BtnDiscord   = TFMMakeButton("tfm_btn_discord",   TFM_BtnStyle.SECONDARY);
        m_BtnWebsite   = TFMMakeButton("tfm_btn_website",   TFM_BtnStyle.SECONDARY);
        m_BtnExit      = TFMMakeButton("tfm_btn_exit",      TFM_BtnStyle.DANGER);

        if (m_TFMActionBar)
        {
            float x, w;
            m_TFMActionBar.GetPos(x, m_TFMBarY);
            m_TFMActionBar.GetSize(w, m_TFMBarH);
        }
    }

    protected TFM_MenuButton TFMMakeButton(string name, TFM_BtnStyle style)
    {
        ButtonWidget w = ButtonWidget.Cast(m_TFMRoot.FindAnyWidget(name));

        if (!w)
        {
            ErrorEx(string.Format("[TFM] кнопка %1 не найдена в main_menu.layout", name));
            return null;
        }

        TFM_MenuButton btn = new TFM_MenuButton(w, style);
        m_TFMButtons.Insert(btn);

        return btn;
    }

    protected void TFMApplyConfig()
    {
        TFMSetText("tfm_status_version", "VERSION " + m_TFMConfig.VersionLabel);
        TFMSetText("tfm_status_server",  "SERVER  " + m_TFMConfig.ServerName);
        TFMSetText("tfm_status_line",    m_TFMConfig.StatusLine);

        // ONLINE / PING заполняются, когда придут данные браузера серверов.
        TFMSetText("tfm_status_online",  "ONLINE  --/--");
        TFMSetText("tfm_status_ping",    "PING    -- ms");
    }

    protected void TFMSetText(string name, string value)
    {
        TextWidget w = TextWidget.Cast(m_TFMRoot.FindAnyWidget(name));

        if (w)
            w.SetText(value);
    }

    //! Прячем ванильную разметку, не трогая её логику.
    protected void TFMHideVanilla(Widget root)
    {
        Widget child = root.GetChildren();

        while (child)
        {
            if (child != m_TFMRoot)
                child.Show(false);

            child = child.GetSibling();
        }
    }

    protected void TFMShowVanilla(Widget root)
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
        if (!m_TFMRoot || !w)
            return super.OnClick(w, x, y, button);

        // Панель раздела перехватывает клики первой
        if (m_TFMSectionOpen && m_TFMSection)
        {
            if (m_TFMSection.OnClick(w))
                return true;

            if (m_TFMSection.GetActionWidget() == w)
            {
                TFMConnectSelected();
                return true;
            }
        }

        // При открытом разделе ИГРАТЬ работает как НАЗАД
        if (TFMIs(w, m_BtnPlay))
        {
            if (m_TFMSectionOpen)
                TFMCloseSection();
            else
                TFMPlay();

            return true;
        }

        if (TFMIs(w, m_BtnServers))   { TFMOpenSection();             return true; }
        if (TFMIs(w, m_BtnCharacter)) { TFMOpen(MENU_CHARACTER);      return true; }
        if (TFMIs(w, m_BtnSettings))  { TFMOpen(MENU_OPTIONS);        return true; }
        if (TFMIs(w, m_BtnDiscord))   { TFMOpenURL(m_TFMConfig.DiscordURL); return true; }
        if (TFMIs(w, m_BtnWebsite))   { TFMOpenURL(m_TFMConfig.WebsiteURL); return true; }
        if (TFMIs(w, m_BtnExit))      { TFMConfirmExit();             return true; }

        return super.OnClick(w, x, y, button);
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (m_TFMSectionOpen && m_TFMSection && m_TFMSection.OnMouseEnter(w))
            return true;

        TFM_MenuButton btn = TFMFind(w);

        if (btn)
        {
            btn.OnMouseEnter();

            // Tooltip в layout закреплён над «СЕРВЕРЫ» (id 5b), поэтому
            // показываем его только для этой кнопки.
            if (btn == m_BtnServers)
                m_TFMTooltip.Show("СПИСОК СЕРВЕРОВ", "Официальные серверы и сообщества");

            return true;
        }

        return super.OnMouseEnter(w, x, y);
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (m_TFMSectionOpen && m_TFMSection && m_TFMSection.OnMouseLeave(w))
            return true;

        TFM_MenuButton btn = TFMFind(w);

        if (btn)
        {
            btn.OnMouseLeave();
            m_TFMTooltip.Hide();
            return true;
        }

        return super.OnMouseLeave(w, enterW, x, y);
    }

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
    {
        TFM_MenuButton btn = TFMFind(w);

        if (btn)
        {
            btn.OnMouseDown();
            return true;
        }

        return super.OnMouseButtonDown(w, x, y, button);
    }

    override bool OnMouseButtonUp(Widget w, int x, int y, int button)
    {
        TFM_MenuButton btn = TFMFind(w);

        if (btn)
        {
            btn.OnMouseUp();
            return true;
        }

        return super.OnMouseButtonUp(w, x, y, button);
    }

    protected bool TFMIs(Widget w, TFM_MenuButton btn)
    {
        return btn && btn.IsEnabled() && btn.GetWidget() == w;
    }

    protected TFM_MenuButton TFMFind(Widget w)
    {
        if (!w || !m_TFMButtons)
            return null;

        foreach (TFM_MenuButton btn : m_TFMButtons)
        {
            if (btn.GetWidget() == w)
                return btn;
        }

        return null;
    }

    // ------------------------------------------------------------- actions

    protected void TFMPlay()
    {
        if (!m_TFMConfig.HasDirectConnect())
        {
            // Сервер не настроен — ведём себя как ваниль.
            TFMOpen(MENU_SERVER_BROWSER);
            return;
        }

        // ВНИМАНИЕ: имя метода прямого коннекта менялось между билдами DayZ.
        // Если компилятор ругается — сверьте с scripts/5_Mission/gui/ServerBrowser.
        g_Game.ConnectFromServerBrowser(m_TFMConfig.ServerIP, m_TFMConfig.ServerPort, m_TFMConfig.ServerPassword);
    }

    // --------------------------------------------------- панель раздела (8c)

    protected void TFMOpenSection()
    {
        if (m_TFMSectionOpen)
            return;

        if (!m_TFMSection)
            m_TFMSection = new TFM_SectionPanel(m_TFMRoot);

        if (!m_TFMSection.GetRoot())
            return;

        m_TFMSection.SetRows(TFMBuildServerRows());
        m_TFMSection.SetActionLabel("ПОДКЛЮЧИТЬСЯ");
        m_TFMSection.Show(true);

        m_TFMSectionOpen = true;

        // лента ужимается до 112, ИГРАТЬ становится НАЗАД, раздел помечен
        TFMSetBarHeight(112.0 / 1080.0);
        TFMSetPlayLabel("НАЗАД");

        if (m_BtnServers)
            m_BtnServers.SetState(TFM_BtnState.ACTIVE, 0.180);
    }

    protected void TFMCloseSection()
    {
        if (!m_TFMSectionOpen)
            return;

        m_TFMSectionOpen = false;

        if (m_TFMSection)
            m_TFMSection.Show(false);

        TFMSetBarHeight(m_TFMBarH);
        TFMSetPlayLabel("ИГРАТЬ");

        if (m_BtnServers)
            m_BtnServers.SetState(TFM_BtnState.NORMAL, 0.120);
    }

    //! Строки раздела «СЕРВЕРЫ». Сейчас — сервер из конфига; сюда же
    //! подставляется реальный список, когда придут данные браузера.
    protected array<ref TFM_SectionRow> TFMBuildServerRows()
    {
        array<ref TFM_SectionRow> rows = new array<ref TFM_SectionRow>;

        rows.Insert(new TFM_SectionRow(
            m_TFMConfig.ServerName,
            "CHERNARUS",
            "-- / --",
            "MILITARY RP",
            "--",
            m_TFMConfig.StatusLine));

        return rows;
    }

    protected void TFMConnectSelected()
    {
        TFM_SectionRow row = m_TFMSection.GetSelected();

        if (!row)
            return;

        TFMPlay();
    }

    protected void TFMSetBarHeight(float height)
    {
        if (!m_TFMActionBar)
            return;

        float x, y, w, h;
        m_TFMActionBar.GetPos(x, y);
        m_TFMActionBar.GetSize(w, h);

        // низ ленты остаётся на месте
        float bottom = m_TFMBarY + m_TFMBarH;

        m_TFMActionBar.SetSize(w, height);
        m_TFMActionBar.SetPos(x, bottom - height);
    }

    protected void TFMSetPlayLabel(string label)
    {
        if (!m_BtnPlay)
            return;

        TextWidget w = TextWidget.Cast(m_BtnPlay.GetWidget().FindAnyWidget("label"));

        if (w)
            w.SetText(label);
    }

    // -------------------------------------------------------- выход (7a)

    protected void TFMConfirmExit()
    {
        TFM_DialogMenu dialog = TFM_DialogMenu.ShowConfirm(
            "ПОДТВЕРЖДЕНИЕ",
            "Вы действительно хотите выйти из игры?",
            "ВЫЙТИ");

        if (!dialog)
        {
            // Диалог не создался — не блокируем выход.
            GetGame().RequestExit(IDC_MAIN_QUIT);
            return;
        }

        dialog.Event_OnResult.Insert(TFMOnExitResult);
    }

    void TFMOnExitResult(bool confirmed)
    {
        if (confirmed)
            GetGame().RequestExit(IDC_MAIN_QUIT);
    }

    protected void TFMOpen(int menuId)
    {
        UIManager ui = GetGame().GetUIManager();

        if (ui)
            ui.EnterScriptedMenu(menuId, this);
    }

    protected void TFMOpenURL(string url)
    {
        if (url == "" || url == "https://")
            return;

        GetGame().OpenURL(url);
    }

    // -------------------------------------------------------------- update

    override void Update(float timeslice)
    {
        super.Update(timeslice);

        if (!m_TFMRoot)
            return;

        m_TFMTime += timeslice;

        TFMUpdateBackground(timeslice);
        TFMUpdateIntro();

        foreach (TFM_MenuButton btn : m_TFMButtons)
            btn.Update(timeslice);

        m_TFMTooltip.Update(timeslice);

        if (m_TFMSectionOpen && m_TFMSection)
            m_TFMSection.Update(timeslice);

        // 5b: пока висит tooltip — фон дополнительно затемняется на 8%
        if (m_TFMDim)
            m_TFMDim.SetColor(TFM_Theme.WithAlpha(TFM_Theme.BG_0, 0.08 * TFMTooltipAlpha()));
    }

    protected float TFMTooltipAlpha()
    {
        if (m_TFMTooltip && m_TFMTooltip.IsVisible())
            return 1.0;

        return 0.0;
    }

    //! Cinematic zoom 8% за 40 s, ping-pong, linear.
    protected void TFMUpdateBackground(float timeslice)
    {
        if (!m_TFMBg)
            return;

        m_TFMBgTime += timeslice;

        float period = TFM_Theme.BG_ZOOM_TIME * 2.0;
        float phase  = m_TFMBgTime - Math.Floor(m_TFMBgTime / period) * period;
        float t      = phase / TFM_Theme.BG_ZOOM_TIME;

        if (t > 1.0)
            t = 2.0 - t;

        float scale = 1.0 + TFM_Theme.BG_ZOOM * t;
        float off   = (scale - 1.0) * 0.5;

        m_TFMBg.SetSize(scale, scale);
        m_TFMBg.SetPos(-off, -off);
    }

    //! Появление меню: логотип 0–240, лента 80–340, статус-бар 140–380 (мс).
    protected void TFMUpdateIntro()
    {
        if (m_TFMTime > 0.400)
            return;

        float ms = m_TFMTime * 1000.0;

        float aLogo   = Math.Clamp((ms -   0.0) / 240.0, 0, 1);
        float aBar    = Math.Clamp((ms -  80.0) / 260.0, 0, 1);
        float aStatus = Math.Clamp((ms - 140.0) / 240.0, 0, 1);

        if (m_TFMLogo)      m_TFMLogo.SetAlpha(aLogo);
        if (m_TFMActionBar) m_TFMActionBar.SetAlpha(aBar);
        if (m_TFMStatusBar) m_TFMStatusBar.SetAlpha(aStatus);
    }
}
