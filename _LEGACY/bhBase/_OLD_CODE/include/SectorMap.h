#pragma once
#include "Sector.h"

////////////////////////////////////////////////////////////////////////////////

class SectorMap
{
public:

	void SetRenderDepth(short iDepth);
	void Render();
	virtual void Update() = 0;
	virtual void MarkVisible() = 0;

protected:

	virtual Sector* GetCamSector() = 0;

	short renderDepth;
	int dim;
	Sector* camSector;

private:
};

////////////////////////////////////////////////////////////////////////////////
