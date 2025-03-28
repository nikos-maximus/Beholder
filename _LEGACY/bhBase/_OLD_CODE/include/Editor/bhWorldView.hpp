#ifndef BH_WORLD_VIEW_H
#define BH_WORLD_VIEW_H

#include "editor/bhWorld_Editor.hpp"
#include "GL/bhDeviceGL.hpp"

class bhWorldView
{
public:
    bool Init(int wndWidth, int wndHeight);
    void Destroy();
    void Display3DView(bool* show, bhTime_t tickTime);
    void DisplayGrid(bool* show);

    bool New()
    {
        world.Destroy();
        return world.New(16, 16);
    }

    bool Load(char const* path)
    {
        world.Destroy();
        return world.Load_Path(path);
    }

    bool CreateFromImage(char const* path)
    {
        world.Destroy();
        return world.CreateFromImage_Path(path);
    }

    bool Save(char const* fileName)
    {
        return world.Save(fileName);
    }

protected:
private:
    bhWorld_Editor world;
    bhResourceID worldViewFB{ BH_INVALID_RESOURCE };
};

#endif //BH_WORLD_VIEW_H
