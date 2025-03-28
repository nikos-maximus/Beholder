#include <SDL3/SDL_assert.h>
#include <string>
#include <glm/ext.hpp>
#include "Game/bhWorld.hpp"
#include "VK/bhDeviceVk.hpp"
#include "VK/bhWorldPipelineVk.hpp"
#include "bhSystem.hpp"
#include "bhInput.hpp"
#include "bhConfig.hpp"
#include "Platform/bhPlatform.hpp"
#include "Mesh/bhMeshCache.hpp"

bhWorld::bhWorld()
{}

bool bhWorld::Init()
{
  int ww, wh;
  bhSystem::WindowSize(ww, wh); // TODO: Error check
  pipeline = bhVk::WorldPipeline::Create(bhVk::GetRenderDevice(), ww, wh);

  bhVk::WorldPipeline::Material::CreateInfo mci;
  mci.textureNames[bhVk::WorldPipeline::Material::SAMPLER_ALBEDO] = "BRICK1.png";
  mci.textureNames[bhVk::WorldPipeline::Material::SAMPLER_NORMAL_SPECULAR] = "BRICK1_normal_specular.png";
  
  bhVk::WorldPipeline::Material* mat = bhVk::WorldPipeline::Material::Create(bhVk::GetRenderDevice(), mci); //TODO: This hould be accessed through cache

  std::string path = std::string(bhPlatform::GetDataDir()) + std::string("\\gltf\\Box\\Box.gltf");
 if (Import(path.c_str()))
  {
    camera = new bhCamera3D();
    camera->SetPosition(10.0f, 10.0f, 10.0f);
    camera->LookAt(0.0f, 0.0f, 0.0f);

    const bhMeshCache::Resource* res = bhMeshCache::Get("Mesh");
    staticMesh = new bhStaticMesh3D(res->deviceMesh, *mat);

    return true;
  }
  return false;
}

void bhWorld::Destroy()
{
  delete staticMesh;
  delete camera;

  bhVk::WorldPipeline::Destroy(bhVk::GetRenderDevice(), pipeline);
}

void bhWorld::HandleEvent(const bhEvent::Event& evt)
{
  switch (evt.type)
  {
    case bhEvent::Type::EVT_FWD:
    {
      //glm::vec3 vd = camera.GetViewData().pos;
      //vd += camera.GetFwdVec() * 0.4f;
      //camera.SetPosition(vd);
      return;
    }
    case bhEvent::Type::EVT_BACK:
    {
      //glm::vec3 vd = camera.GetViewData().pos;
      //vd -= camera.GetFwdVec() * 0.4f;
      //camera.SetPosition(vd);
      return;
    }
    case bhEvent::Type::EVT_LEFT:
    {
      //glm::vec3 vd = camera.GetViewData().pos;
      //vd -= camera.GetRightVec() * 0.4f;
      //camera.SetPosition(vd);
      return;
    }
    case bhEvent::Type::EVT_RIGHT:
    {
      //glm::vec3 vd = camera.GetViewData().pos;
      //vd += camera.GetRightVec() * 0.4f;
      //camera.SetPosition(vd);
      return;
    }
    case bhEvent::Type::EVT_MOUSE_MOVE:
    {
      const bhConfig::InputSettings& is = bhSystem::Config()->inputSt;
      float yaw = (float)evt.data.mouseMotion.xrel * is.mouse_sensitivity;
      float pitch = (float)evt.data.mouseMotion.yrel * is.mouse_sensitivity;
      //camera.Yaw(-yaw);
      //camera.Pitch(-pitch);
      return;
    }
    default:
    {
      return;
    }
  }
}

void bhWorld::Tick(bhTime_t deltaTime)
{}

void bhWorld::Render()
{
  bhVk::WorldPipeline::Bind(bhVk::GetRenderDevice(), *pipeline);
  bhVk::WorldPipeline::SetCameraView(bhVk::GetRenderDevice(), *pipeline, camera->GetTransform(), camera->GetProjection());
  staticMesh->Render();
}

bool bhWorld::Save(const char* filePath)
{
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "w");
  if (!err)
  {
    // TODO: Report error
    return false;
  }
  SDL_assert(file); // If the above check passes, this should never fail

  fclose(file);
  return true;
}

bool bhWorld::Load(const char* filePath)
{
  FILE* file = nullptr;
  errno_t err = fopen_s(&file, filePath, "r");
  if (!err)
  {
    // TODO: Report error
    return false;
  }
  SDL_assert(file); // If the above check passes, this should never fail

  fclose(file);
  return true;
}
