#pragma once
#include "SectorMap.h"

////////////////////////////////////////////////////////////////////////////////

class SectorPlane : public SectorMap
{
public:

	SectorPlane(unsigned short exp);
	~SectorPlane();
	Sector* GetSector(int x,int z);
	void Update();

protected:

	Sector* GetCamSector();
	Sector** sectors;

private:
};

////////////////////////////////////////////////////////////////////////////////
