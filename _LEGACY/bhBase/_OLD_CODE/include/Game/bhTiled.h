#pragma once

namespace tinyxml2
{
    class XMLElement;
}

class bhTiled
{
public:
    bool Load(char const* path);

protected:
    enum Orientation
    {
        ORIENTATION_ORTHOGONAL,
        ORIENTATION_ISOMETRIC,
        ORIENTATION_STAGGERED,
        ORIENTATION_HEXAGONAL
    };

    void LoadTileset(tinyxml2::XMLElement const* tilesetEl);
    void LoadLayer(tinyxml2::XMLElement const* tilesetEl);

private:
};
