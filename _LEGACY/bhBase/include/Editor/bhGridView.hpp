#ifndef BH_WORLD_EDITOR_HPP
#define BH_WORLD_EDITOR_HPP

class bhWorld;

class bhGridView
{
public:
    bhGridView(bhWorld& _world);
    bool New(int xdim, int ydim);
    bool CreateFromImage(char const* path);
    void SetPlayerPosition(int x, int y);
    void Display(bool* show);
    bool Import(const char* filePath) const;

protected:
private:
    bhGridView() = delete;

    bhWorld& world;
};

#endif //BH_WORLD_EDITOR_HPP
