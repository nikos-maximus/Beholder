#ifndef BH_MESH_HPP
#define BH_MESH_HPP

#include <vector>
#include "bhVec2.hpp"
#include "bhVec3.hpp"

class bhMesh
{
public:
	using Index_t = uint32_t;

	struct Patch
	{
		uint32_t baseVertex { 0 }, numVerts { 0 };
		uint32_t baseIndex { 0 }, numInds { 0 };
	};

	struct Vertex
	{
		bhVec3f position, normal, tangent;
		bhVec2f uv0;
	};

	struct PatchCreateInfo
	{
		const std::vector<Vertex>* verts { nullptr };
		const std::vector<Index_t>* inds { nullptr };
	};

	bool Create(const std::vector<std::vector<Vertex>>& vertsPerPatch, const std::vector<std::vector<Index_t>>& indsPerPatch);

protected:
private:
	std::vector<Vertex> verts;
	std::vector<Index_t> inds;
	std::vector<Patch> patches;
};

#endif //BH_MESH_HPP
