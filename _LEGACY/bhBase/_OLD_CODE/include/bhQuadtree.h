#pragma once
#include "bhScene.h"

class bhCamera;

class bhQuadtree
{
public:

	bhQuadtree(float _extent, float _leafExtent, float _vertExtent);

	void AddMeshComponent(bhMeshComponent* mc);
	void Render(bhCamera const* cam, std::vector<bhLight*> const* lights);

protected:

	struct Node
	{
		static const uint8_t NUM_CHILDREN = 4;

		Node();
		~Node();
		void CreateChildren(float leafExtent, float vertExtent);
		void AddMeshComponent(bhMeshComponent* mc, glm::vec3 const& pos);
		void Render(bhBox const& lightArea);
		void RenderGeomOnly();

		Node* children[NUM_CHILDREN];
		bhBox box;
		bhSector* sector;
	};

	Node root;
	float extent, leafExtent, vertExtent;

private:
};
