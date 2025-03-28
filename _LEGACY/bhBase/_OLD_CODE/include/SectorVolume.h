#pragma once
#include "SectorMap.h"

////////////////////////////////////////////////////////////////////////////////

class SectorVolume : public SectorMap
{
public:

	SectorVolume(unsigned short exp);
	~SectorVolume();
	Sector* GetSector(int x,int y,int z);
	void Update();

protected:

	Sector* GetCamSector();
	Sector*** sectors;

private:
};

////////////////////////////////////////////////////////////////////////////////
