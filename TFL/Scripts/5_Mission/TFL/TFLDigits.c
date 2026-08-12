// Крупное табличное число из текстур-глифов.
//
// Подтверждённые шрифтовые ресурсы игры дают 22px, а хендофф требует
// таймер 92px и позицию в очереди 88px. Поэтому крупные цифры набираются
// не текстом, а картинками: слоты разложены в .layout (digit_row), здесь
// им подставляются текстуры.
//
// Число выравнивается по правому краю: у слотов одинаковая ширина, так что
// «7» и «37» стоят на одной линии — это и есть tabular-nums из спеки.

class TFL_Digits
{
    static const string DIR = "TFL/GUI/textures/digits/";

    protected ref array<ImageWidget> m_Slots;
    protected ImageWidget m_Separator;

    void TFL_Digits(Widget root, string name, int count)
    {
        m_Slots = new array<ImageWidget>;

        if (!root)
            return;

        for (int i = 0; i < count; i++)
        {
            ImageWidget slot = ImageWidget.Cast(
                root.FindAnyWidget(string.Format("%1_%2", name, i)));

            if (slot)
                m_Slots.Insert(slot);
        }

        m_Separator = ImageWidget.Cast(root.FindAnyWidget(name + "_sep"));
    }

    //! value — только цифры, например "07" или "142".
    void SetValue(string value)
    {
        int count = m_Slots.Count();
        int len   = value.Length();

        for (int i = 0; i < count; i++)
        {
            ImageWidget slot = m_Slots.Get(i);

            // правое выравнивание: заполняем с конца
            int index = i - (count - len);

            if (index < 0)
            {
                slot.Show(false);
                continue;
            }

            slot.Show(true);
            slot.LoadImageFile(0, DIR + "d" + value.Substring(index, 1) + ".paa");
        }
    }

    void SetColor(int color)
    {
        foreach (ImageWidget slot : m_Slots)
            slot.SetColor(color);

        if (m_Separator)
            m_Separator.SetColor(color);
    }

    void SetAlpha(float alpha)
    {
        foreach (ImageWidget slot : m_Slots)
            slot.SetAlpha(alpha);

        if (m_Separator)
            m_Separator.SetAlpha(alpha);
    }

    void Show(bool show)
    {
        foreach (ImageWidget slot : m_Slots)
            slot.Show(show);

        if (m_Separator)
            m_Separator.Show(show);
    }
}
