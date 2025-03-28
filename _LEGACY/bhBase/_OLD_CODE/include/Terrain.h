#ifndef BH_TERRAIN_H
#define BH_TERRAIN_H

#include <glm/glm.hpp>

////////////////////////////////////////////////////////////////////////////////

class Camera;

class Terrain
{
public:

	Terrain();
	~Terrain();
	bool LoadHeightMapImage(char const* name);
	bool Load();
	bool Save();
	void Update();
	void Render(const Camera* cam);

	inline void SetUnitSize(float iUnitSize)
	{
		unitSz = iUnitSize;
	}

	inline void SetHeight(float iMinHeight,float iMaxHeight)
	{
		minHeight = iMinHeight;
		maxHeight = iMaxHeight;
	}

protected:

	struct HeightMapData
	{
		glm::vec3 pos;
		glm::vec2 txCoord;
		glm::vec3 normal;
	};

	void Generate();

	short sizeExp;
	float unitSz;
	float minHeight,maxHeight;
	HeightMapData** heightMap;

private:
};

////////////////////////////////////////////////////////////////////////////////

#endif
