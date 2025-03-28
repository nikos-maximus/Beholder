#ifndef BH_SCENENODE_H
#define BH_SCENENODE_H

#include <vector>

////////////////////////////////////////////////////////////////////////////////

class Camera;

class SceneNode
{
public:

	virtual void Update();
	virtual void Render(const Camera* cam);
	//virtual bool IsVisible(const Camera* cam) = 0;
	//virtual bool Contains(const Vector4f& pos) = 0;
	//virtual float GetVolume() = 0;

	void AddChild(SceneNode* newchild)
	{
		children.push_back(newchild);
		newchild->parent = this;
	}

	void SetParent(SceneNode* iparent)
	{
		if(!parent)
		{
			return;
		}
		parent = iparent;
	}

	Vec4f GetPos()
	{
		return position;
	}

	void SetPosition(const Vec4f& iPos)
	{
		position = iPos;
	}

	void SetVisible(bool enable)
	{
		enable ? (flags |= FL_VISIBLE) : (flags &= ~FL_VISIBLE);
	}

	void Enable(bool enable)
	{
		enable ? (flags |= FL_ENABLED) : (flags &= ~FL_ENABLED);
	}

protected:

	////////////////////////////////////////////////////////////////////////////////
	enum Flags
	{
		FL_ENABLED = 1 << 0,
		FL_VISIBLE = 1 << 1
	};
	////////////////////////////////////////////////////////////////////////////////

	SceneNode();
	virtual ~SceneNode(void);

	short flags;
	Vec4f position;
	SceneNode* parent;
	std::vector<SceneNode*> children;

private:
};

////////////////////////////////////////////////////////////////////////////////
