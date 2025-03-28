#ifndef BH_RENDERER_GL_HPP
#define BH_RENDERER_GL_HPP

#include "GL/bhStructsGL.hpp"
#include "GL/bhPipelineGL.hpp"
#include "Private/bhGPUDevice.hpp"
#include "Private/bhResourceManager.hpp"
#include "bhWindow.h"

////////////////////////////////////////////////////////////////////////////////
class bhDeviceGL : public bhGPUDevice
{
public:
	void BeginFrame() override final;
	void EndFrame() override final;
	bhResourceID CreateTextureFromImage(const bhImage* image, const char* name, int32_t offsetx, int32_t offsety) override final;

	bhResourceID CreateFramebuffer(bhSize2Di size, uint32_t numColorAttachments, bool createDepthStencil) override final;
	void DestroyFramebuffer(bhResourceID fbId) override final;
	void UseFramebuffer(bhResourceID fbId) override final;
	void UseWindowFramebuffer() override final;
	void* GetFramebufferColorAttachment(bhResourceID fbId, size_t attachmentIdx) override final;
	bhSize2Di GetFramebufferSize(bhResourceID fbId) override final;

	bhBufferGL CreateBuffer(GLenum target, GLenum usage, GLsizeiptr reqSize);
	void DestroyBuffer(bhBufferGL* pBuffer);
	bool CopyDataToBuffer(const bhBufferGL* pBuffer, GLintptr offset, GLsizei reqSize, const void* data);

	const bhTextureGL* GetTextureObject(bhResourceID textureID)
	{
		return textureManager.GetResourceObject(textureID);
	}

	bhResourceID CreatePipeline_World() override;
	bool IsPipelineValid_World(bhResourceID pipelineID) override;
	void UsePipeline_World(bhResourceID pipelineID) override;
	void DestroyPipeline_World(bhResourceID pipelineID) override;
	
	bhResourceID CreateMaterial_World(bhResourceID pipelineID, const bhWorldMaterialCreateInfo* materialCI) override;
	void UseMaterial_World(bhResourceID materialID) override;
	void DestroyMaterial_World(bhResourceID materialID) override;
	
	void UploadMesh_World(bhResourceID pipelineID, bhResourceID meshID) override;
	void RenderMesh_World(bhResourceID pipelineID, bhResourceID meshID, const glm::mat4* transform) override;
	void SetCameraView_World(bhResourceID pipelineID, const bhCamera::ViewData* viewProjection) override;

#ifdef BH_USE_IMGUI
	bool InitImGui() override;
	void DestroyImGui() override;
	void BeginImGuiFrame() override;
	void EndImGuiFrame() override;
#endif //BH_USE_IMGUI

protected:
	bool Init() override final;
	void Destroy() override final;

private:
	static GLFormats PickImageFormat(const bhImage* img);
	static void DestroyTexture(bhTextureGL* texture);
	static float GetViewportAspect();
	static GLenum MapImageTypeToOpenGL(int dim);
	static bool IsTextureValid(const bhTextureGL* texture);
	
	static bhTextureGL* CreateTexture(const bhImage* img, GLint border, GLFormats glFmt);
	static bhTextureGL* CreateTexture(const bhImage* img, GLint border)
	{
		return CreateTexture(img, border, PickImageFormat(img));
	}

	void GenerateMipmaps(bhResourceID textureID);

	bhTextureInputGL bhWorldMaterial_SetupTextureInput(const std::string* textureName, bhSamplerSlots slot);
	void UseMaterial_World(const bhMaterialGL_World* mat);
	void bhWorldPipeline_SetupVAO(bhPipelineGL_World* pipeline, int bindingsMask);
	void bhWorldPipeline_CreateDataBuffers(bhPipelineGL_World* pipeline);
	void bhWorldPipeline_DestroyDataBuffers(bhPipelineGL_World* pipeline);

	bhResourceManager<bhTextureGL> textureManager;
	
	SDL_GLContext GLContext{ NULL };
};

#endif //BH_RENDERER_GL_HPP
