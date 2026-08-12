// 12 подсказок с финальным текстом (карточка 8a).
// Порядок и формулировки — из дизайн-хендоффа, менять только вместе с ним.

class TFL_Hint
{
    string Category;
    string Title;
    string Description;
    string Image;   //!< путь к .paa; пустой — слот IMAGE скрывается

    void TFL_Hint(string category, string title, string description, string image = "")
    {
        Category    = category;
        Title       = title;
        Description = description;
        Image       = image;
    }
}

class TFL_Hints
{
    private static ref array<ref TFL_Hint> s_Hints;

    static array<ref TFL_Hint> Get()
    {
        if (s_Hints)
            return s_Hints;

        s_Hints = new array<ref TFL_Hint>;

        s_Hints.Insert(new TFL_Hint("ВОЕННАЯ ТЕХНИКА", "Проверка транспорта",
            "Перед использованием транспорта убедитесь в его технической исправности и наличии топлива."));

        s_Hints.Insert(new TFL_Hint("РАДИОСВЯЗЬ", "Частоты фракций",
            "Используйте назначенные вашей фракции радиочастоты. Открытые каналы прослушиваются."));

        s_Hints.Insert(new TFL_Hint("РОЛЕВАЯ ИГРА", "Достоверность действий",
            "Действия персонажа должны соответствовать происходящей ситуации."));

        s_Hints.Insert(new TFL_Hint("РАБОТЫ", "Гражданские профессии",
            "Профессии доступны через специальных NPC в городах."));

        s_Hints.Insert(new TFL_Hint("МЕДИЦИНА", "Тяжёлые ранения",
            "Тяжёлые ранения требуют квалифицированной медицинской помощи."));

        s_Hints.Insert(new TFL_Hint("ТРАНСПОРТ", "Служебная техника",
            "Не оставляйте служебную технику без необходимости."));

        s_Hints.Insert(new TFL_Hint("ФРАКЦИИ", "Структура организаций",
            "Каждая организация имеет собственную структуру, иерархию и правила."));

        s_Hints.Insert(new TFL_Hint("БАЗЫ", "Ограниченный доступ",
            "Некоторые военные территории имеют ограниченный доступ."));

        s_Hints.Insert(new TFL_Hint("ПРАВИЛА", "Регламент сервера",
            "Перед началом игры ознакомьтесь с правилами сервера."));

        s_Hints.Insert(new TFL_Hint("ВЗАИМОДЕЙСТВИЕ", "Сначала — разговор",
            "Используйте ролевое взаимодействие перед применением силы."));

        s_Hints.Insert(new TFL_Hint("ОРУЖИЕ", "Состояние вооружения",
            "Следите за износом и чистотой используемого вооружения."));

        s_Hints.Insert(new TFL_Hint("РОЛЕВАЯ ИГРА", "Мир реагирует",
            "Ваши действия влияют на развитие игрового мира."));

        return s_Hints;
    }
}
