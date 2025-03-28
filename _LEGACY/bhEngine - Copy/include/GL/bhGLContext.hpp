#ifndef BH_GL_CONTEXT_HPP
#define BH_GL_CONTEXT_HPP

#include "GL/bhGLIncludes.hpp"
#include "bhGPUContext.hpp"

class bhGLContext : public bhGPUContext
{
public:
  void PreCreateWindow() override final;
  Uint32 GetWindowFlag() override final;
  bool Init(SDL_Window* wnd) override final;
  void Destroy() override final;
  void BeginFrame() override final;
  void EndFrame() override final;

  bool InitImGui(SDL_Window* wnd) override final;
  void DestroyImGui() override final;

  bool UploadWorldMesh(bhWorldPipeline* pl, bhMesh* mesh) override final;

  bhWorldPipeline* CreateWorldPipeline() override final;
  void BindWorldPipeline(const bhWorldPipeline* pl) override final;
  void SetWorldCameraView(bhWorldPipeline* pl, const bhCamera::ViewData* vd) override final;
  void DrawWorldMesh(const bhWorldPipeline* pl, const bhMesh* mesh, const glm::mat4& transform) override final;

  bhWorldMaterial* LoadWorldMaterial(const char* filename) override final;
  void DestroyWorldMaterial(bhWorldMaterial* mat) override final;
  void BindWorldMaterial(const bhWorldMaterial* mat) override final;

protected:
private:
  void* sdlGLContext{ nullptr };
  SDL_Window* window{ nullptr };
};

#endif //BH_GL_CONTEXT_HPP
