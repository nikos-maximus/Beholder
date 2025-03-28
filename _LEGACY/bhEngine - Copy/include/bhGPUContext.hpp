#ifndef BH_GPU_CONTEXT_HPP
#define BH_GPU_CONTEXT_HPP

#include <SDL3/SDL_stdinc.h>
#include "bhPipeline.hpp"
#include "bhCamera.hpp"

struct SDL_Window;
class bhMesh;

class bhGPUContext
{
public:
  virtual ~bhGPUContext() {}

  virtual void PreCreateWindow() = 0;
  virtual Uint32 GetWindowFlag() = 0;
  virtual bool Init(SDL_Window* wnd) = 0;
  virtual void Destroy() = 0;
  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual bool InitImGui(SDL_Window* wnd) = 0;
  virtual void DestroyImGui() = 0;

  virtual bool UploadWorldMesh(bhWorldPipeline* pl, bhMesh* mesh) = 0;

  virtual bhWorldPipeline* CreateWorldPipeline() = 0;
  virtual void BindWorldPipeline(const bhWorldPipeline* pl) = 0;
  virtual void SetWorldCameraView(bhWorldPipeline* pl, const bhCamera::ViewData* vd) = 0;
  virtual void DrawWorldMesh(const bhWorldPipeline* pl, const bhMesh* mesh, const glm::mat4& transform) = 0;

  virtual bhWorldMaterial* LoadWorldMaterial(const char* filename) = 0;
  virtual void DestroyWorldMaterial(bhWorldMaterial* mat) = 0;
  virtual void BindWorldMaterial(const bhWorldMaterial* mat) = 0;

protected:
  bhGPUContext() = default;

private:
};

#endif //BH_GPU_CONTEXT_HPP
