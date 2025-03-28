#include "Game/bhDungeon.h"
#include "Math/bhMath.h"
#include "bhUtil.h"
#include "bhImage.h"

//DEBUG
//#include <stdio.h>
//DEBUG

static const int MIN_ROOM_DIM = 2;

////////////////////////////////////////////////////////////////////////////////
struct Node
{
    ~Node()
    {
        delete max;
        delete min;
    }

    void Split(int level)
    {
        if (level % 2 > 0)
        {
        }
        //min = new Node(level + 1);
        //max = new Node(level + 1);
    }

    Sector sector;
    Node* min = nullptr;
    Node* max = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
class Tree
{
public:
    Tree(int xextent, int yextent)
        : root(new Sector(0, 0, xextent, yextent))
    {}

    ~Tree()
    {
        delete root;
    }

protected:
private:
    Sector* root = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
bhDungeon::bhDungeon()
    : bhGrid()
{}

bhDungeon::bhDungeon(int _xdim, int _ydim)
    : bhGrid(_xdim, _ydim)
{}

inline int Displacement()
{
    const int DISPLACEMENT = 2;
    return (rand() % (DISPLACEMENT * 2 + 1)) - DISPLACEMENT;
}

void bhDungeon::SplitSector(Sector const& sector, bool splitOnXAxis)
{
    int sectorXSiz = sector.xmax - sector.xmin;
    int sectorYSiz = sector.ymax - sector.ymin;

    if (splitOnXAxis)
    {
        if (sectorXSiz <= MIN_ROOM_DIM) return;
    }
    else
    {
        if (sectorYSiz <= MIN_ROOM_DIM) return;
    }

    //int split_x = rand() % (sectorXSiz - 2) + 1;
    //int split_y = rand() % (sectorYSiz - 2) + 1;

    if (splitOnXAxis)
    {
        int split_x = rand() % sectorXSiz;
        split_x = bhMath::Clamp(split_x, MIN_ROOM_DIM, sectorXSiz - MIN_ROOM_DIM - 1);
        for (int y = 0; y < sectorYSiz; ++y)
        {
            // (y == split_y) continue;
            SetBlock(sector.xmin + split_x, sector.ymin + y, 1); // > 0 means solid
        }

        //DEBUG_Print();

        Sector leftSector(sector.xmin, sector.ymin, sector.xmin + split_x, sector.ymax);
        SplitSector(leftSector, !splitOnXAxis);
        
        Sector rightSector(sector.xmin + split_x + 1, sector.ymin, sector.xmax, sector.ymax);
        SplitSector(rightSector, !splitOnXAxis);
    }
    else
    {
        int split_y = rand() % sectorYSiz;
        split_y = bhMath::Clamp(split_y, MIN_ROOM_DIM, sectorYSiz - MIN_ROOM_DIM - 1);
        for (int x = 0; x < sectorXSiz; ++x)
        {
            //if (x == split_x) continue;
            SetBlock(sector.xmin + x, sector.ymin + split_y, 1); // > 0 means solid
        }

        //DEBUG_Print();

        Sector lowerSector(sector.xmin, sector.ymin, sector.xmax, sector.ymin + split_y);
        SplitSector(lowerSector, !splitOnXAxis);
        
        Sector upperSector(sector.xmin, sector.ymin + split_y + 1, sector.xmax, sector.ymax);
        SplitSector(upperSector, !splitOnXAxis);
    }
}

void bhDungeon::Generate_RecursiveSplit()
{
    int rowOffset = 0;
    for (int row = 0; row < ydim; ++row)
    {
        for (int col = 0; col < xdim; ++col)
        {
            blocks[rowOffset + col] = 0; // > 0 means solid
            ++numWalls;
        }
        rowOffset += xdim;
    }

    // Pick a split point
    Sector r(0, 0, xdim, ydim);
    SplitSector(r, true);
}

void Erode(float* srcValues, float* dstValues, int e_xdim, int e_ydim)
{
    int e_rowOffset = 0;
    for (int row = 1; row < e_ydim - 1; ++row)
    {
        e_rowOffset = row * e_xdim;
        for (int col = 1; col < e_xdim - 1; ++col)
        {
            float val = (
                srcValues[e_rowOffset + col] +
                srcValues[e_rowOffset + col - 1] +
                srcValues[e_rowOffset + col + 1] +
                srcValues[e_rowOffset - e_xdim + col] +
                srcValues[e_rowOffset + e_xdim + col]) / 5.f;

            dstValues[e_rowOffset + col] = bhMath::Min(val, srcValues[e_rowOffset + col]);
        }
    }
}

void bhDungeon::Generate_Erosion()
{
    int e_xdim = xdim + 2;
    int e_ydim = ydim + 2;
    float* values0 = static_cast<float*>(calloc(e_xdim * e_ydim, sizeof(float)));
    float* values1 = static_cast<float*>(calloc(e_xdim * e_ydim, sizeof(float)));

    int e_rowOffset = 0;
    for (int row = 0; row < e_ydim; ++row)
    {
        for (int col = 0; col < e_xdim; ++col)
        {
            values0[e_rowOffset + col] = (rand() % (e_xdim + e_ydim) < 3) ? 0.f : 1.f;
            ++numWalls;
        }
        e_rowOffset += e_xdim;
    }

    for (int i = 0; i < 12; ++i)
    {
        Erode(values0, values1, e_xdim, e_ydim);
        bhUtil::Swap(values0, values1);
    }

    int rowOffset = 0;
    for (int row = 0; row < ydim; ++row)
    {
        e_rowOffset = e_xdim * (row + 1);
        for (int col = 0; col < xdim; ++col)
        {
            blocks[rowOffset + col] = (values1[e_rowOffset + col + 1] < 0.5f) ? 0 : 1; // > 0 means solid
            ++numWalls;
        }
        rowOffset += xdim;
    }
    free(values0);
    free(values1);
}

bool bhDungeon::Generate_FromImage(char const* fileName)
{
    delete[] blocks;
    blocks = nullptr;

    bhImage image;
    if (!bhImage::CreateFromFile(image, fileName))
    {
        return false;
    }
    bhImageInfo const& info = image.GetInfo();
    xdim = info.width;
    ydim = info.height;
    blocks = new int[xdim * ydim];

    unsigned char const* pixels = image.GetPixels();
    unsigned char const* currPixel = nullptr;

    int rowOffset = 0;
    for (int row = 0; row < ydim; ++row)
    {
        for (int col = 0; col < xdim; ++col)
        {
            currPixel = &(pixels[(rowOffset + col) * info.numComponents]);
            if (*currPixel == 0)
            {
                blocks[rowOffset + col] = 1;
                ++numWalls;
            }
            else
            {
                blocks[rowOffset + col] = 0;
            }
        }
        rowOffset += xdim;
    }
    return true;
}

#if 0
void bhDungeon::DEBUG_Print()
{
    for (int row = ydim - 1; row >= 0; --row)
    {
        for (int col = 0; col < xdim; ++col)
        {
            //printf("%03d|", initValues[rowOffset + col]);
            //printf("%s", initValues[rowOffset + col] > (MAX_VAL >> 1) ? "@" : " ");
            switch (GetBlock(col, row))
            {
                case 0:
                {
                    putchar(' ');
                    putchar(' ');
                    break;
                }
                case 1:
                default:
                {
                    putchar(219);
                    putchar(219);
                    break;
                }
            }
        }
        printf("\n");
    }
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}
#endif
