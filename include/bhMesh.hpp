#ifndef BH_MESH_HPP
#define BH_MESH_HPP

#include <vector>
//#include <stdint.h>
#include <bhVec2.hpp>
#include <bhVec3.hpp>

class bhMesh
{
public:
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
		const std::vector<bhVec3f>* positions { nullptr };
		const std::vector<bhVec3f>* normals { nullptr };
		const std::vector<bhVec3f>* tangents { nullptr };
		const std::vector<bhVec2f>* uv0 { nullptr };
		//const std::vector<bhVec2f>* uv1 { nullptr };

		const std::vector<uint32_t>* inds { nullptr };
	};

	bool Create(const std::vector<PatchCreateInfo>& gci);

protected:
private:
	std::vector<Vertex> verts;
	std::vector<uint32_t> inds;
	std::vector<Patch> patches;
};

#endif //BH_MESH_HPP
