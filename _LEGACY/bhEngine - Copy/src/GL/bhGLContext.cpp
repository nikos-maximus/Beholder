#include <SDL3/SDL_video.h>
#include "GL/bhGLContext.hpp"
#include "GL/bhGL.hpp"
#include "bhSystem.hpp"
#include "bhConfig.hpp"
#include "Mesh/bhMesh.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

//DEBUG
#include <assert.h>
//DEBUG

void bhGLContext::PreCreateWindow()
{
  const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, rs.gl.versionMajor);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, rs.gl.versionMinor);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#ifdef _DEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
}

Uint32 bhGLContext::GetWindowFlag()
{
  return SDL_WINDOW_OPENGL;
}

bool bhGLContext::Init(SDL_Window* wnd)
{
  window = wnd;
  sdlGLContext = SDL_GL_CreateContext(window);
  
  if (gl3wInit())
  {
    return false;
  }

  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
  glClearDepth(1.0);

  //glFrontFace(GL_CCW); // Default
  glCullFace(GL_BACK);
  //glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_LINE);

  return GL_TRUE;
}

void bhGLContext::Destroy()
{
  SDL_GL_DeleteContext(sdlGLContext);
}

void bhGLContext::BeginFrame()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bhGLContext::EndFrame()
{
  glFlush();
  //glFinish(); // For benchmarking?
  SDL_GL_SwapWindow(window);
}

bool bhGLContext::UploadWorldMesh(bhWorldPipeline* pl, bhMesh* mesh)
{
  //GLsizei reqSize = mesh->numVerts * sizeof(bhWorldVertex);
  //bhGL::CopyDataToBuffer(&(pl->vertexBuffer), pl->vertexBufferWriteOffset, reqSize, mesh->verts);
  //pl->vertexBufferWriteOffset += reqSize;
  //mesh->offsets.offsetsGL.baseVertex = pl->baseVertex;
  //pl->baseVertex += mesh->numVerts;

  //reqSize = mesh->numInds * sizeof(bhMeshIdx_t);
  //bhGL::CopyDataToBuffer(&(pl->indexBuffer), pl->indexBufferWriteOffset, reqSize, mesh->inds);
  //pl->indexBufferWriteOffset += reqSize;
  //mesh->offsets.offsetsGL.baseIndex = pl->baseIndex;
  //pl->baseIndex += mesh->numInds;

  assert(false);
  return false;
}

bool bhGLContext::InitImGui(SDL_Window* wnd)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  if (ImGui_ImplSDL3_InitForOpenGL(wnd, sdlGLContext))
  {
    const bhConfig::RenderSettings& rs = bhSystem::Config()->renderSt;
    static const size_t VERSION_SRING_LEN = 16;
    char versionString[VERSION_SRING_LEN];
    sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", rs.gl.versionMajor, rs.gl.versionMinor);
    if (ImGui_ImplOpenGL3_Init(versionString))
    {
      ImGui::StyleColorsDark();
      return true;
    }
  }
  return false;
}

void bhGLContext::DestroyImGui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

bhWorldPipeline* bhGLContext::CreateWorldPipeline()
{
  return nullptr;
}

void bhGLContext::BindWorldPipeline(const bhWorldPipeline* pl)
{}

bhWorldMaterial* bhGLContext::LoadWorldMaterial(const char* filename)
{
  return nullptr;
}

void bhGLContext::DestroyWorldMaterial(bhWorldMaterial* mat)
{}

void bhGLContext::BindWorldMaterial(const bhWorldMaterial* mat)
{}

void bhGLContext::SetWorldCameraView(bhWorldPipeline* pl, const bhCamera::ViewData* vd)
{}

void bhGLContext::DrawWorldMesh(const bhWorldPipeline* pl, const bhMesh* mesh, const glm::mat4& transform)
{}
