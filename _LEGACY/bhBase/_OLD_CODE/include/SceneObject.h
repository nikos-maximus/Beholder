#ifndef BH_SCENEOBJECT_H
#define BH_SCENEOBJECT_H

#include "Mesh.h"
#include "Material.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

class SceneObject
{
public:

	SceneObject();
	SceneObject(Mesh* cMesh,Material* cMaterial);
	//~SceneObject();
	void Render();
	void RenderSelection();
	
	inline void SetMesh(Mesh* iMesh)
	{
		mesh = iMesh;
	}

	inline void SetMaterial(Material* iMaterial)
	{
		material = iMaterial;
	}

	void Update();

	void Move(glm::vec3 const& v,unsigned short frameCount = 0);
	inline void Move(float x,float y,float z,unsigned short frameCount = 0)
	{
		Move(glm::vec3(x,y,z),frameCount);
	}

	void MoveTo(glm::vec3 const& v,unsigned short frameCount = 0);
	inline void MoveTo(float x,float y,float z,unsigned short frameCount = 0)
	{
		MoveTo(glm::vec3(x,y,z),frameCount);
	}

	//void SetSelectionId(unsigned char itemId,unsigned char groupId = 1,unsigned char subGroupId = 1);

	//bool Load();
	//bool Save();

	inline void SetScale(const glm::vec3& v)
	{
		scale = v;
	}

	inline void SetScale(float x,float y,float z)
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;
	}

	inline glm::vec3 GetPosition() const
	{
		return position;
	}

	void SetVisible(bool enable)
	{
		enable ? (flags |= SO_VISIBLE) : (flags &= ~SO_VISIBLE);
	}

	bool IsVisible()
	{
		return (flags & SO_VISIBLE) != 0;
	}

protected:

	enum{
		SO_ENABLED = 1,
		SO_VISIBLE = 2,
		SO_SELECTABLE = 4
	};

	struct Animation
	{
		Animation(int frameCount):framesLeft(frameCount){}

		int framesLeft;
		glm::vec3 speed;
		glm::vec3 accel;
	};

	short flags;
	Material* material;
	Mesh* mesh;
	glm::vec3 position;
	glm::vec3 scale;
	std::vector<Animation> animations;

	//TODO
	//Animations must be erased when finished -> use list?

private:
};

//void ClearSepectableObjects();

};
////////////////////////////////////////////////////////////////////////////////
#endif
