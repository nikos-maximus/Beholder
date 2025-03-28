#ifndef BH_WORLD_HPP
#define BH_WORLD_HPP

#include "bhDefines.h"
#include "bhCamera.hpp"
#include "bhEvent.hpp"

class bhMesh;

////////////////////////////////////////////////////////////////////////////////
enum bhGridDirection
{
  BH_GRID_EAST,
  BH_GRID_NORTH,
  BH_GRID_WEST,
  BH_GRID_SOUTH,

  BH_GRID_BOTTOM,
  BH_GRID_TOP,

  BH_NUM_GRID_DIRECTIONS
};

////////////////////////////////////////////////////////////////////////////////
class bhWorld
{
public:

  static constexpr uint8_t MAX_MAP_DIM = 16;

  bhWorld();
  bool Init();
  void Destroy();
  void HandleEvent(const bhEvent::Event& evt);
  //void RegisterCommandHandlers();
  void Tick(bhTime_t deltaTime);
  void Render();

  //bool Load_Resource(const char* fileName);
  //bool Load_Path(const char* path);

  //glm::vec3 GetPlayerPosition() const
  //{
  //	return player.GetPosition();
  //}

  //glm::vec3 GetPlayerGridPosition() const
  //{
  //	glm::vec3 pos = player.GetPosition();
  //	pos.x /= blockSize.side;
  //	pos.y /= blockSize.side;
  //	pos.z /= blockSize.height;
  //	return pos;
  //}

  //void SetupBlock(int x, int y, bool setupNeighbors);
  //void ClearBlock(int x, int y);

  //void PlayerMove_Hold(bhCommandID cmd);
  //void PlayerTurn_Hold(bhCommandID cmd);

  void GetDimensions(int& x, int& y) const;
  bool IsBlockSolid(int x, int y, int z = 1) const;
  bool SetBlockSolid(int x, int y, bool solid);
  bool Import(const char* filePath);

protected:
  struct BlockSize
  {
    float side{ 2.0f };
    float height{ 2.0f };
  }
  blockSize;

  //bhBlockPlayer player;
  //void CreateResources();
  //void DestroyResources();

  bool AreCoordsValid(int x, int y, int z) const
  {
    return (x >= 0) && (x < MAX_MAP_DIM) && (y >= 0) && (y < MAX_MAP_DIM) && (z >= 0) && (z < 3);
  }

private:
  struct MapBlock
  {
    static void SetupRotations();

    static glm::mat4 rotations[BH_NUM_GRID_DIRECTIONS];
    //glm::vec3 translations[BH_NUM_GRID_DIRECTIONS];
    bhMesh* wallMeshes[BH_NUM_GRID_DIRECTIONS] = {};
    int flags{ 1 }; // Zero for empty space
  };

  //bool SaveBlock(FILE* file, const MapBlock& blk, const bhResourceManager<struct bhMesh>* worldMeshManager);
  //bool LoadBlock(FILE* file, MapBlock& blk);

  //////////////////////////////////////////////////////////////////////////////////
  bool Save(const char* filePath);
  bool Load(const char* filePath);
  bool UpdateBlock(int x, int y, int z = 1);
  glm::vec3 GetWallTranslation(int x, int y, int z, bhGridDirection dir);

  //static void MeshDeleteFunc(struct bhMesh* mesh);

  //bhResourceID worldPipeline{ BH_INVALID_RESOURCE };
  //static const uint8_t MAX_MESHES{ 2 };
  //bhResourceID meshIDs[MAX_MESHES];
  //static const uint8_t MAX_MATERIALS{ 2 };
  //bhResourceID materialIDs[MAX_MATERIALS];

  //bhResourceManager<struct bhMesh> meshManager;

  static constexpr uint16_t BLOCKS_DELIM{ UINT16_MAX };
  MapBlock blocks[MAX_MAP_DIM][MAX_MAP_DIM][3] = {};

  bhCamera camera;
  bhMesh* DEBUG_testMesh{ nullptr };
  glm::mat4x4 xForm;

  static glm::mat4 identity;
};

#endif //BH_WORLD_HPP
