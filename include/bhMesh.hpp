#ifndef BH_MESH_HPP
#define BH_MESH_HPP

#include <vector>
#include "bhVec2.hpp"
#include "bhVec3.hpp"

class bhMesh
{
public:
	using Index_t = uint16_t;

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

	const size_t GetVertsSiz() const { return verts.size() * sizeof(Vertex); }
	const Vertex* GetVertsData() const { return verts.data(); }

	const size_t GetIndsSiz() const { return inds.size() * sizeof(Index_t); }
	const Index_t* GetIndsData()const { return inds.data(); }

	void*& GetApiImpl() { return apiImpl; }

protected:
private:
	std::vector<Vertex> verts;
	std::vector<Index_t> inds;
	std::vector<Patch> patches;
	void* apiImpl { nullptr };
};

#endif //BH_MESH_HPP
