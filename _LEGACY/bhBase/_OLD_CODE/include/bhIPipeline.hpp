#ifndef BH_IPIPELINE_HPP
#define BH_IPIPELINE_HPP

#include "bhCamera.hpp"

////////////////////////////////////////////////////////////////////////////////
class bhIPipeline
{
public:
protected:
	bhIPipeline() = default;
	virtual ~bhIPipeline() = default;

private:
};

////////////////////////////////////////////////////////////////////////////////
class bhIWorldPipeline
{
public:
	virtual void SetCameraView(const bhCamera::ViewData* viewProjection) = 0;
	virtual void RenderMesh(const bhDeviceMesh* deviceMesh, const glm::mat4* transform);

protected:
	bhIWorldPipeline() = default;

private:
};

////////////////////////////////////////////////////////////////////////////////

struct bhWorldMaterial;
struct bhWorldPipeline;
struct bhDeviceMesh;
struct bhModelData;

void DestroyMaterial_World(bhWorldMaterial* material);
void UseMaterial_World(const bhWorldMaterial* mat);

bhWorldPipeline CreatePipeline_World();
bhWorldMaterial* CreateMaterial_World(const bhWorldPipeline* /*pipeline*/, const bhWorldMaterialCreateInfo* materialCI);
void DestroyPipeline_World(bhWorldPipeline* pipeline);
void UsePipeline_World(const bhWorldPipeline* pipeline);

void UploadMesh_World(bhWorldPipeline* pipeline, bhDeviceMesh* deviceMesh);

//void bhWorldPipeline_UploadInstanceData(const bhWorldPipeline* pipeline, const bhModelData* modelData, size_t numInstances);

#endif //BH_IPIPELINE_HPP
