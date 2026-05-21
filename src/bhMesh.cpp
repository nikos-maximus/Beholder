#include <SDL3/SDL_assert.h>
#include "bhMesh.hpp"

bool bhMesh::Create(const std::vector<std::vector<Vertex>>& vertsPerPatch, const std::vector<std::vector<Index_t>>& indsPerPatch)
{
	size_t numPatces = vertsPerPatch.size();
	if (numPatces != indsPerPatch.size())
	{
		SDL_assert(false);
		return false;
	}

	uint32_t numTotalVerts = 0, numTotalInds = 0;
	for (size_t p = 0; p < numPatces; ++p)
	{
		if (vertsPerPatch[p].empty())
		{
			SDL_assert(false);
			return false;
		}

		SDL_assert(vertsPerPatch[p].size() <= size_t(UINT32_MAX));
		uint32_t numPatchVerts = static_cast<uint32_t>(vertsPerPatch[p].size());

		if (indsPerPatch[p].empty())
		{
			SDL_assert(false);
			return false;
		}

		SDL_assert(indsPerPatch[p].size() <= size_t(UINT32_MAX));
		uint32_t numPatchInds = static_cast<uint32_t>(indsPerPatch[p].size());

		Patch newPatch;
		newPatch.baseVertex = numTotalVerts;
		newPatch.numVerts = numPatchVerts;
		newPatch.baseIndex = numTotalInds;
		newPatch.numInds = numPatchInds;
		patches.push_back(newPatch);

		numTotalVerts += numPatchVerts;
		numTotalInds += numPatchInds;
	}

	verts.resize(numTotalVerts);
	inds.resize(numTotalInds);

	for (size_t p = 0; p < patches.size(); ++p)
	{
		const Patch& currP = patches[p];
		memcpy(&(verts[currP.baseVertex]), vertsPerPatch[p].data(), currP.numVerts * sizeof(Vertex));
		memcpy(&(inds[currP.baseIndex]), indsPerPatch[p].data(), currP.numInds * sizeof(Index_t));
	}

	return true;
}
