#ifndef BH_CONFIG_UI_H
#define BH_CONFIG_UI_H

#include <vector>
#include <SDL.h>
#include "bhConfig.h"

class bhConfigUI
{
public:
    ~bhConfigUI();
    bool Init();
    void Display(bool* show);

protected:
private:
    void OnSave();

    struct DisplayData
    {
        std::vector<SDL_DisplayMode> displayModes_v;
        std::vector<char*> displayModeStrings_v;
    };

    std::vector<const char*> displayNames_v;
    std::vector<DisplayData> displayData_v;
    
    //std::vector< std::vector<SDL_DisplayMode> > displayModeSets_v;
    //std::vector< std::vector<char*> > displayModeStringSets;

    bhRenderSettings rs;
    bhWindowSettings ws;

    int currDisplayMode = 0;
};

#endif //BH_CONFIG_UI_H
