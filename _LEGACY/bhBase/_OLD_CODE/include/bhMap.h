#pragma once
#include <glm/glm.hpp>
#include "bhDefs.h"

#define MAP_SIZ 32
#define TILE_SZ 4.0f

extern const glm::vec3 g_WorldUpVec;

namespace bhMap
{
	bool Destroy();
	bool Load(char const* name);
	void Update(float timeDelta, double mouseRelx, double mouseRely);
	void Render();
	void HandleCommands(uint8_t const* commands);

	// Editor

	enum SectorTypes : uint8_t
	{
		ST_SOLID,
		ST_EMPTY,
		ST_DOOR,

		NUM_SECTOR_TYPES
	};

	char const* GetSectorTypeName(SectorTypes typeEnum);

	struct FileData
	{
		FileData();
		void Clear();
		bool Load(char const* path);
		bool Save(char const* path);

		char wallMatName[ASSET_NAME_LEN];
		char floorMatName[ASSET_NAME_LEN];
		char ceilMatName[ASSET_NAME_LEN];
		uint8_t rawTiles[MAP_SIZ][MAP_SIZ];
		int8_t lastSelectionX, lastSelectionY;
	};

	void RenderEditorGrid(FileData const* fd);
	//bool Save(char const* name);

	enum MapSelectionMode
	{
		SEL_DEACTIVATE,
		SEL_ACTIVATE
	};

	void EditorSelectTile(FileData* fd, float x, float y, MapSelectionMode mode);
	//void New();
	void Create();
};
