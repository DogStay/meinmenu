// Конфиг главного меню. Лежит в $profile:TFM_MENUGAME_UI/menu_config.json,
// создаётся с дефолтами при первом запуске — правится без пересборки PBO.

class TFM_MenuConfig
{
    string  ServerName      = "THE FIRST LINE | MILITARY RP";
    string  ServerIP        = "127.0.0.1";
    int     ServerPort      = 2302;
    string  ServerPassword  = "";
    string  DiscordURL      = "https://discord.gg/";
    string  WebsiteURL      = "https://";
    string  StatusLine      = "THE FIRST LINE | MILITARY RP  ·  CHERNARUS";
    string  VersionLabel    = "1.0.0";

    //! Центрированная раскладка загрузки (id 8b) — для 3440×1440 и 4K.
    bool    CenteredLoading = false;

    static const string DIR  = "$profile:TFM_MENUGAME_UI";
    static const string PATH = "$profile:TFM_MENUGAME_UI/menu_config.json";

    private static ref TFM_MenuConfig s_Instance;

    static TFM_MenuConfig Get()
    {
        if (!s_Instance)
            s_Instance = Load();

        return s_Instance;
    }

    static TFM_MenuConfig Load()
    {
        TFM_MenuConfig cfg = new TFM_MenuConfig();

        if (FileExist(PATH))
        {
            JsonFileLoader<TFM_MenuConfig>.JsonLoadFile(PATH, cfg);
        }
        else
        {
            if (!FileExist(DIR))
                MakeDirectory(DIR);

            JsonFileLoader<TFM_MenuConfig>.JsonSaveFile(PATH, cfg);
        }

        return cfg;
    }

    bool HasDirectConnect()
    {
        return ServerIP != "" && ServerIP != "127.0.0.1" && ServerPort > 0;
    }
}
