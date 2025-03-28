#include "bhQuadtree.h"
#include "bhLight.h"
#include "bhCamera.h"
#include "bhRendering.h"

bhQuadtree::Node::Node()
	: sector(0)
{
	memset(children, 0, NUM_CHILDREN * sizeof(Node*));
}

bhQuadtree::Node::~Node()
{
	for (uint8_t c = 0; c < NUM_CHILDREN; ++c)
	{
		delete children[c];
	}
	delete sector;
}

void bhQuadtree::Node::CreateChildren(float leafExtent, float vertExtent)
{
	if (box.halfSiz.x > leafExtent)
	{
		glm::vec3 childHalfSiz(box.halfSiz.x * 0.5f, box.halfSiz.y * 0.5f, vertExtent);
		for (uint8_t c = 0; c < NUM_CHILDREN; ++c)
		{
			children[c] = new Node();
			children[c]->box.halfSiz = childHalfSiz;
			children[c]->box.center.x = (c & BIT(0)) ? (box.center.x - childHalfSiz.x) : (box.center.x + childHalfSiz.x);
			children[c]->box.center.y = (c & BIT(1)) ? (box.center.y - childHalfSiz.y) : (box.center.y + childHalfSiz.y);
			children[c]->CreateChildren(leafExtent, vertExtent);
		}
	}
	else
	{
		sector = new bhSector();
		sector->box.center = box.center;
		sector->box.halfSiz = box.halfSiz;
	}
}

void bhQuadtree::Node::RenderGeomOnly()
{
	if (sector)
	{
		for (auto c : sector->components)
		{
			bhMeshComponent::RenderGeomOnly(&c);
		}
	}
	else
	{
		for (uint8_t c = 0; c < NUM_CHILDREN; ++c)
		{
			children[c]->RenderGeomOnly();
		}
	}
}

void bhQuadtree::Node::Render(bhBox const& lightArea)
{
	if (box.DoesNotOverlap(lightArea))
	{
		return;
	}
	if (sector)
	{
		for (auto c : sector->components)
		{
			bhMeshComponent::Render(&c);
		}
	}
	else
	{
		for (uint8_t c = 0; c < NUM_CHILDREN; ++c)
		{
			children[c]->Render(lightArea);
		}
	}
}

void bhQuadtree::Node::AddMeshComponent(bhMeshComponent* mc, glm::vec3 const& pos)
{
	if (box.Contains(pos))
	{
		if (sector)
		{
			sector->components.push_back(*mc);
		}
		else
		{
			for (uint8_t c = 0; c < NUM_CHILDREN; ++c)
			{
				children[c]->AddMeshComponent(mc, pos);
			}
		}
	}
}

bhQuadtree::bhQuadtree(float _extent, float _leafExtent, float _vertExtent)
	: extent(_extent)
	, leafExtent(_leafExtent)
	, vertExtent(_vertExtent)
{
	root.box.halfSiz = glm::vec3(extent);
	root.CreateChildren(leafExtent, vertExtent);
}

void bhQuadtree::AddMeshComponent(bhMeshComponent* mc)
{
	root.AddMeshComponent(mc, glm::vec3(mc->transform[3][0], mc->transform[3][1], mc->transform[3][2]));
}

void bhQuadtree::Render(bhCamera const* cam, std::vector<bhLight*> const* lights)
{
	GLuint prog = 0;
	GLint mvpULocation = 0;

	bhRendering::UseProgram("EarlyDepth");
	bhTransform::QueryUniforms();
	cam->SetViewVMP_Only();

	glClear(GL_DEPTH_BUFFER_BIT);
	root.RenderGeomOnly();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	bhRendering::UseProgram("Scene");
	bhTransform::QueryUniforms();
	cam->SetView();

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	for (auto ls : *lights)
	{
		if (!ls->IsActive())
		{
			continue;
		}
		bhBox lightArea(ls->GetPosition(), ls->GetRadius());
		ls->Render();
		root.Render(lightArea);
	}

	glDisable(GL_BLEND);
}
