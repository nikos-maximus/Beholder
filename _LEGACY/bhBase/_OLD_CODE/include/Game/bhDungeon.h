#pragma once
#include "bhGrid.h"

////////////////////////////////////////////////////////////////////////////////
struct Sector
{
    // Sector coordinates are inclusive. Meaning:
    // xmin = a xmax = b --> [a,b]
    Sector(int _xpos, int _ypos, int _xextent, int _yextent)
        : xmin(_xpos), ymin(_ypos), xmax(_xextent), ymax(_yextent)
    {}

    int xmin = 0, ymin = 0, xmax = 0, ymax = 0;
};

////////////////////////////////////////////////////////////////////////////////
class bhDungeon : public bhGrid<int>
{
public:
    bhDungeon();
    bhDungeon(int _xdim, int _ydim);

    void Generate_RecursiveSplit();
    void Generate_Erosion();
    bool Generate_FromImage(char const* fileName);
    
    //void DEBUG_Print();
    int GetNumWalls() const 
    { 
        return numWalls; 
    }

protected:
    void SplitSector(Sector const& sector, bool splitOnXAxis);

private:
    int numWalls = 0;
};
