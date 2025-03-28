#pragma once
#include "Types.h"

namespace GameTech
{

class SPTree
{
public:

	enum SPTreeType
	{
		SPT_BINARY = 1,
		SPT_QUADTREE = 2,
		SPT_OCTREE = 3
	};

	SPTree(SPTreeType iType,unsigned short iDepth);
	~SPTree();

protected:

	struct SPTreeNode
	{
		SPTreeNode()
			:childrenIndex(0)
		{}

		unsigned int childrenIndex;
	};

	void AssignNodeChildrenInds();

	inline unsigned int GetLevelNodes(unsigned short level)
	{
		return 1 << (grade * level);
	}

	unsigned int GetTotalNodes();

	unsigned short grade,depth;
	
	//grade: the total children of each node in the tree is 2^grade, or 1 << grade
	//for example: quadtree is of grade 2 --> each node has 2^2 = 4 children
	//SPTreeType actually encodes the grade

private:

	std::vector<SPTreeNode> nodes;
};

}; //end GameTech