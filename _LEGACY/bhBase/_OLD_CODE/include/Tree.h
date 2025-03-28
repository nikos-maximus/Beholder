#ifndef BH_TREE_H
#define BH_TREE_H

#include <vector>

////////////////////////////////////////////////////////////////////////////////

class bhTree
{
public:

	struct Properties
	{
		float rootRange;
		float ratio;
		float shrinkFactor;
		short branchFactor;
		//GTMath::vec3<float> rootUp;
	};

	bhTree(const Properties* p);
	~bhTree();
	void Create();

protected:

	struct Branch
	{
		Branch(const Branch* parent);
		void CreateBranches(float rootRange);

		Branch* parent;
		std::vector<Branch*> chidren;
	};

	Branch* root;
	Properties* tp;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
