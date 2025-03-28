#include "Game/bhTiled.h"
#include <tinyxml2.h>

void bhTiled::LoadTileset(tinyxml2::XMLElement const* tilesetEl)
{
    char const* name = tilesetEl->Attribute("name");
    int tilewidth = tilesetEl->IntAttribute("tilewidth");
    int tileheight = tilesetEl->IntAttribute("tileheight");
    int spacing = tilesetEl->IntAttribute("spacing");
    int margin = tilesetEl->IntAttribute("margin");
    int tilecount = tilesetEl->IntAttribute("tilecount");
    int columns = tilesetEl->IntAttribute("columns");
}

void bhTiled::LoadLayer(tinyxml2::XMLElement const* layerEl)
{
    using namespace tinyxml2;

    int id = layerEl->IntAttribute("id");
    char const* name = layerEl->Attribute("name");
    int width = layerEl->IntAttribute("width");
    int height = layerEl->IntAttribute("height");

    XMLElement const* dataEl = layerEl->FirstChildElement("data");
    char const* encoding = dataEl->Attribute("encoding");
    char const* compression = dataEl->Attribute("compression");
    if (!encoding && !compression)
    {
        // Plain XML
        XMLElement const* tileEl = dataEl->FirstChildElement("tile");
        while (tileEl)
        {
            int gid = tileEl->IntAttribute("gid");
            tileEl = tileEl->NextSiblingElement();
        }
    }
    if (compression)
    {
        if (strcmp(compression, "zlib") == 0)
        {
        }
        if (strcmp(compression, "gzip") == 0)
        {
        }
        if (strcmp(compression, "zstd") == 0)
        {
        }
    }
    if (encoding)
    {
        if (strcmp(encoding, "csv") == 0)
        {
            char const* values = dataEl->GetText();
            size_t valuesLen = strlen(values) + 1;
            char* tmpValues = reinterpret_cast<char*>(calloc(valuesLen, sizeof(char)));
            strcpy(tmpValues, values);

            char* token = strtok(tmpValues, ",");
            while (token)
            {
                int index = atoi(token); // Each index is a new gid
                token = strtok(nullptr, ",");
            }

            free(tmpValues);
        }
        if (strcmp(encoding, "base64") == 0)
        {
        }
    }
}

bool bhTiled::Load(char const* path)
{
    using namespace tinyxml2;

    XMLDocument doc;
    XMLError err = doc.LoadFile(path);
    if (err)
    {
        return false;
    }
    XMLElement const* rootEl = doc.RootElement(); // The <map> element
    float version = rootEl->FloatAttribute("version");
    float tiledversion = rootEl->FloatAttribute("tiledversion");
    char const* orientation = rootEl->Attribute("orientation");
    //renderorder string right-down / right-up / left-down / left-up
    int width = rootEl->IntAttribute("width");
    int height = rootEl->IntAttribute("height");
    int tilewidth = rootEl->IntAttribute("tilewidth");
    int tileheight = rootEl->IntAttribute("tileheight");
    bool infinite = rootEl->BoolAttribute("infinite");
    //nextlayerid int
    //nextobjectid int

    // Handle properties
    XMLElement const* propertiesEl = rootEl->FirstChildElement("properties");
    while (propertiesEl)
    {
        XMLElement const* propertyEl = propertiesEl->FirstChildElement("property");
        while (propertyEl)
        {
            propertyEl = propertyEl->NextSiblingElement();
        }
        propertiesEl = propertiesEl->NextSiblingElement();
    }

    // Handle tilesets
    /*
    XMLElement const* tilesetEl = rootEl->FirstChildElement("tileset");
    while (tilesetEl)
    {
        int firstgid = tilesetEl->IntAttribute("firstgid");
        char const* source = tilesetEl->Attribute("source");
        if (source)
        {
            // Load external tileset file
            XMLDocument tilesetDoc;
            XMLError err = tilesetDoc.LoadFile(path);
            if (err)
            {
                return false;
            }
            XMLElement const* tilesetRoot = tilesetDoc.RootElement();
            LoadTileset(tilesetRoot);
        }
        else
        {
            LoadTileset(tilesetEl);
        }
        tilesetEl = tilesetEl->NextSiblingElement();
    }
    */

    // Handle layers
    XMLElement const* layerEl = rootEl->FirstChildElement("layer");
    while (layerEl)
    {
        LoadLayer(layerEl);
        layerEl = layerEl->NextSiblingElement();
    }
    return true;
}
