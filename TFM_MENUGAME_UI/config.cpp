class CfgPatches
{
    class TFM_MENUGAME_UI
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Scripts"};
    };
};

class CfgMods
{
    class TFM_MENUGAME_UI
    {
        dir = "TFM_MENUGAME_UI";
        hideName = 1;
        hidePicture = 1;
        name = "THE FIRST LINE | MILITARY RP - UI";
        type = "mod";
        dependencies[] = {"Mission"};

        class defs
        {
            class missionScriptModule
            {
                value = "";
                files[] = {"TFM_MENUGAME_UI/Scripts/5_Mission"};
            };
        };
    };
};
