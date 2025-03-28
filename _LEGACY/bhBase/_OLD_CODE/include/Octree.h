#ifndef BH_OCTREE_H
#define BH_OCTREE_H

#include "Box.h"

////////////////////////////////////////////////////////////////////////////////

class bhOctree
{
public:

	bhOctree(unsigned short cDepth,bhBox cVol);
	~bhOctree();

protected:

	void Create(bhBox const* vol,unsigned short level,int index);

	bhBox** boxNodes;
	unsigned short depth;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
