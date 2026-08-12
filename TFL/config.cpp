class CfgPatches
{
    class TFL
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data","DZ_Scripts"};
    };
};

class CfgMods
{
    class TFL
    {
        dir = "TFL";
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
                files[] = {"TFL/Scripts/5_Mission"};
            };
        };
    };
};
