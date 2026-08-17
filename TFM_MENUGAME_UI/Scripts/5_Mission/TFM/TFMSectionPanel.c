// Панель раздела (id 8c) — открывается поверх затемнённого главного меню.
// Выбранная строка: olive-заливка 10% + левая марка 3px.

class TFM_SectionRow
{
    string Name;
    string Map;
    string Players;
    string Mode;
    string Ping;
    string Description;

    void TFM_SectionRow(string name, string map, string players, string mode, string ping, string description = "")
    {
        Name        = name;
        Map         = map;
        Players     = players;
        Mode        = mode;
        Ping        = ping;
        Description = description;
    }
}

class TFM_SectionPanel
{
    static const string LAYOUT   = "TFM_MENUGAME_UI/GUI/Layouts/section_panel.layout";
    static const int    MAX_ROWS = 6;

    protected Widget            m_Root;
    protected TextWidget        m_Desc;
    protected ref TFM_MenuButton m_Action;

    protected ref array<Widget>          m_Rows;
    protected ref array<ref TFM_SectionRow> m_Data;
    protected int m_Selected;

    void TFM_SectionPanel(Widget parent)
    {
        m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT, parent);

        if (!m_Root)
        {
            ErrorEx("[TFM] section_panel.layout не загрузился");
            return;
        }

        m_Desc     = TextWidget.Cast(m_Root.FindAnyWidget("tfm_section_desc"));
        m_Rows     = new array<Widget>;
        m_Selected = -1;

        for (int i = 0; i < MAX_ROWS; i++)
        {
            Widget row = m_Root.FindAnyWidget(string.Format("tfm_row_%1", i));

            if (row)
            {
                m_Rows.Insert(row);
                row.Show(false);
            }
        }

        ButtonWidget action = ButtonWidget.Cast(m_Root.FindAnyWidget("tfm_section_action"));

        if (action)
            m_Action = new TFM_MenuButton(action, TFM_BtnStyle.PRIMARY);
    }

    void ~TFM_SectionPanel()
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

    Widget GetActionWidget()
    {
        if (m_Action)
            return m_Action.GetWidget();

        return null;
    }

    int GetSelectedIndex()
    {
        return m_Selected;
    }

    TFM_SectionRow GetSelected()
    {
        if (!m_Data || m_Selected < 0 || m_Selected >= m_Data.Count())
            return null;

        return m_Data.Get(m_Selected);
    }

    void SetActionLabel(string label)
    {
        if (!m_Action)
            return;

        TextWidget w = TextWidget.Cast(m_Action.GetWidget().FindAnyWidget("label"));

        if (w)
            w.SetText(label);
    }

    void SetRows(array<ref TFM_SectionRow> rows)
    {
        m_Data     = rows;
        m_Selected = -1;

        for (int i = 0; i < m_Rows.Count(); i++)
        {
            Widget row = m_Rows.Get(i);
            bool   has = rows && i < rows.Count();

            row.Show(has);

            if (!has)
                continue;

            TFM_SectionRow data = rows.Get(i);

            SetCell(row, "name",    data.Name);
            SetCell(row, "map",     data.Map);
            SetCell(row, "players", data.Players);
            SetCell(row, "mode",    data.Mode);
            SetCell(row, "ping",    data.Ping);

            ApplyRowStyle(row, false);
        }

        if (rows && rows.Count() > 0)
            Select(0);
    }

    void Select(int index)
    {
        if (!m_Data || index < 0 || index >= m_Data.Count() || index >= m_Rows.Count())
            return;

        if (m_Selected >= 0 && m_Selected < m_Rows.Count())
            ApplyRowStyle(m_Rows.Get(m_Selected), false);

        m_Selected = index;

        ApplyRowStyle(m_Rows.Get(index), true);

        if (m_Desc)
            m_Desc.SetText(m_Data.Get(index).Description);
    }

    bool OnClick(Widget w)
    {
        for (int i = 0; i < m_Rows.Count(); i++)
        {
            if (m_Rows.Get(i) == w)
            {
                Select(i);
                return true;
            }
        }

        return false;
    }

    bool OnMouseEnter(Widget w)
    {
        if (m_Action && m_Action.GetWidget() == w)
        {
            m_Action.OnMouseEnter();
            return true;
        }

        return false;
    }

    bool OnMouseLeave(Widget w)
    {
        if (m_Action && m_Action.GetWidget() == w)
        {
            m_Action.OnMouseLeave();
            return true;
        }

        return false;
    }

    void Update(float timeslice)
    {
        if (m_Action)
            m_Action.Update(timeslice);
    }

    protected void SetCell(Widget row, string cell, string value)
    {
        TextWidget w = TextWidget.Cast(row.FindAnyWidget(cell));

        if (w)
            w.SetText(value);
    }

    protected void ApplyRowStyle(Widget row, bool selected)
    {
        Widget fill = row.FindAnyWidget("fill");
        Widget mark = row.FindAnyWidget("mark");

        if (fill)
            fill.SetColor(selected ? TFM_Theme.WithAlpha(TFM_Theme.OLIVE_DARK, 0.10) : TFM_Theme.PANEL);

        if (mark)
            mark.SetColor(selected ? TFM_Theme.OLIVE : TFM_Theme.TRANSPARENT);
    }
}
