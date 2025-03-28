#pragma once
#include <vector>
#include "bhDefs.h"
#include "bhBox.h"
#include "bhLight.h"
#include "bhRendering.h"

class bhMesh;
class bhMaterial;

namespace bhTransform
{
	void QueryUniforms();
}

//==============================================================================
// bhMeshComponent

class bhMeshComponent
{
public:

	bhMeshComponent();
	bhMeshComponent(bhMesh* _mesh, bhMaterial* _mat);
	void Translate(float x, float y, float z);
	void Translate(glm::vec3 const& t);
	void Rotate(glm::vec3 const& axis,float degrees);
	static void RenderEarlyZ(bhMeshComponent* mc);
	static void Render(bhMeshComponent* mc);

protected:

	glm::mat4 transform;
	bhMesh* mesh;
	bhMaterial* mat;

private:
};

inline void bhMeshComponent::Translate(float x, float y, float z)
{
	transform[3][0] += x;
	transform[3][1] += y;
	transform[3][2] += z;
}

inline void bhMeshComponent::Translate(glm::vec3 const& t)
{
	Translate(t.x, t.y, t.z);
}

//==============================================================================
// bhSector

class bhSector
{
public:

	bhSector(glm::vec3 const& _center, glm::vec3 const& _size);
	void AddMeshComponent(bhMeshComponent& cmp);
	std::vector<bhMeshComponent> const& GetMeshComponents() const;
	bool Overlaps(bhBox const& b) const;
	void ClearComponents();

protected:

	bhBox box;
	std::vector<bhMeshComponent> components;

private:
};

inline void bhSector::AddMeshComponent(bhMeshComponent& cmp)
{
	cmp.Translate(box.center);
	components.push_back(cmp);
}

inline std::vector<bhMeshComponent> const& bhSector::GetMeshComponents() const
{
	return components;
}

inline bool bhSector::Overlaps(bhBox const& b) const
{
	return box.Overlaps(b);
}

inline void bhSector::ClearComponents()
{
	components.clear();
}
