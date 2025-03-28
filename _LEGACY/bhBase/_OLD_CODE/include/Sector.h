#pragma once

////////////////////////////////////////////////////////////////////////////////

class Sector
{
public:

	enum NeighborIndex
	{
		NEIGHBOR_NORTH,
		NEIGHBOR_SOUTH,
		NEIGHBOR_EAST,
		NEIGHBOR_WEST,
		NEIGHBOR_OVER,
		NEIGHBOR_UNDER,

		MAX_NEIGHBORS
	};

	Sector();
	void Update();
	void Render(short depth);
	void SetRendering();
	void UnsetRendering();

protected:

	enum Flags
	{
		SCTR_MARK_FOR_RENDERING = 1,
		SCTR_DONE_RENDERING = 2
	};

	Sector* neighbors[MAX_NEIGHBORS];
	short flags;

private:
};

////////////////////////////////////////////////////////////////////////////////
