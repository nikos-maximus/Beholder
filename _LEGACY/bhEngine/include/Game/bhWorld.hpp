#ifndef BH_WORLD_HPP
#define BH_WORLD_HPP

#include <memory>
#include "bhDefines.h"
#include "Scene/bhCamera3D.hpp"
#include "Scene/bhStaticMesh3D.hpp"
#include "bhEvent.hpp"

struct cgltf_asset;

namespace bhVk
{
  struct WorldPipeline;
}

class bhWorld
{
public:
  bhWorld();
  bool Init();
  void Destroy();
  void HandleEvent(const bhEvent::Event& evt);
  void Tick(bhTime_t deltaTime);
  void Render();
  bool Import(const char* filePath);

protected:
private:
  bool Save(const char* filePath);
  bool Load(const char* filePath);
  void LogAsset(const cgltf_asset& ast);

  bhVk::WorldPipeline* pipeline{ nullptr };

  bhCamera3D* camera{ nullptr };
  bhStaticMesh3D* staticMesh{ nullptr };
};

#endif //BH_WORLD_HPP
