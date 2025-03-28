#ifndef BH_QUADTREE_H
#define BH_QUADTREE_H

#include "Rect.h"

////////////////////////////////////////////////////////////////////////////////

class bhQuadtree
{
public:

	bhQuadtree(unsigned short cDepth,Recti cArea);
	~bhQuadtree();

protected:

	void Create(Recti const* area,unsigned short level,int index);

	Recti** rectNodes;
	unsigned short depth;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
