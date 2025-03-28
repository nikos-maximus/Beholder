#ifndef BH_EDITOR_HPP
#define BH_EDITOR_HPP

#include <memory>
#include "Editor/bhConfigUI.hpp"
#include "Editor/bhGridView.hpp"
//#include "Editor/bhResourceBrowser.hpp"

struct SDL_Window;
class bhWorld;

////////////////////////////////////////////////////////////////////////////////
class bhEditor
{
public:
    bhEditor(bhWorld& _world);
    void HandleSDLEvents();
    void Render();

protected:
private:
    bhEditor() = delete;
    void DisplayMenu();

    void OnFileNew();
    void OnFileOpen();
    void OnFileCreateFromImage();
    void OnFileSave();
    void OnFileExit();
    void OnFileExitEditor();
    void OnImportGeometries();

    bool showConfig{ false };
    std::unique_ptr<bhConfigUI> configUI;

    bool showGridView{ false };
    std::unique_ptr<bhGridView> gridView;
    
    bool showResourceBrowser{ false };
    //bhResourceBrowser resourceBrowser;

    //bhWorldView worldView;
    //bool showWorldGrid{ true };
    //bool showWorld3D{ true };

    bool showDemo{ false };
};

#endif //BH_EDITOR_HPP
