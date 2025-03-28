#ifndef SPNODE_H
#define SPNODE_H

#include "Box.h"

////////////////////////////////////////////////////////////////////////////////

class Camera;
class SceneNode;

class SPNode
{
public:

	static unsigned short minNodeSizeExponent;

protected:

	void DeleteChildren(unsigned short numChildren);
	float GetDist(const GTMath::vec3<float>& v);
	float GetFastDist(const GTMath::vec3<float>& v);
	SPNode* GetContainingNode(const GTMath::vec3<float>& v,unsigned short numChildren) const;
	SPNode* GetContainingNode(const Box<float>& ibb,unsigned short numChildren) const;
	bool AssignSceneNode(SceneNode* iSceneNode,unsigned short numChildren);
	void Render(const Camera* cam,unsigned short numChildren);

	inline GTMath::vec3<float> GetCenter()
	{
		return bb.pos + (bb.siz / (float)2);
	}

	bool leaf;
	SPNode* parent;
	SPNode** children;
	SceneNode* sceneNode;
	Box<float> bb;

private:
};

////////////////////////////////////////////////////////////////////////////////

class QuadtreeNode : public SPNode
{
public:

	QuadtreeNode(SPNode* iparent,unsigned short edgeExponent,float unitSize,const GTMath::vec3<float>& pos);
	virtual ~QuadtreeNode();
	
	inline void Render(const Camera* cam)
	{
		SPNode::Render(cam,QUAD_CHILDREN);
	}

	inline QuadtreeNode* GetContainingNode(const GTMath::vec3<float>& v) const
	{
		return (QuadtreeNode*)SPNode::GetContainingNode(v,QUAD_CHILDREN);
	}

	inline QuadtreeNode* GetContainingNode(const Box<float>& ibb) const
	{
		return (QuadtreeNode*)SPNode::GetContainingNode(ibb,QUAD_CHILDREN);
	}

	inline bool AssignSceneNode(SceneNode* iSceneNode)
	{
		return SPNode::AssignSceneNode(iSceneNode,QUAD_CHILDREN);
	}

protected:

	static const unsigned short QUAD_CHILDREN = 4;

private:
};

////////////////////////////////////////////////////////////////////////////////

class OctreeNode : public SPNode
{
public:

	OctreeNode(SPNode* iparent,unsigned short edgeExponent,float unitSize,const GTMath::vec3<float>& pos);
	virtual ~OctreeNode();

	inline void Render(const Camera* cam)
	{
		SPNode::Render(cam,OCT_CHILDREN);
	}
	
	inline OctreeNode* GetContainingNode(const GTMath::vec3<float>& v) const
	{
		return (OctreeNode*)SPNode::GetContainingNode(v,OCT_CHILDREN);
	}

	inline OctreeNode* GetContainingNode(const Box<float>& ibb) const
	{
		return (OctreeNode*)SPNode::GetContainingNode(ibb,OCT_CHILDREN);
	}

	inline bool AssignSceneNode(SceneNode* iSceneNode)
	{
		return SPNode::AssignSceneNode(iSceneNode,OCT_CHILDREN);
	}

protected:
	
	static const unsigned short OCT_CHILDREN = 8;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
