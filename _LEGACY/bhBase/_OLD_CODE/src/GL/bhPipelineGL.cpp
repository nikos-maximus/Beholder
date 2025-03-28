#include "GL/bhPipelineGL.hpp"
#include "GL/bhDeviceGL.hpp"
#include "bhImage.h"
#include "bhUtil.h"
#include "bhEnv.h"
#include "bhMesh.hpp"

//static bhBufferGL g_instanceDataBuffer;

////////////////////////////////////////////////////////////////////////////////
inline bool bhWorldMaterial_IsTextureInputValid(const bhTextureInputGL* ti)
{
    return
        (ti->texture.id > 0) &&
        ((ti->texture.target == GL_TEXTURE_1D) || (ti->texture.target == GL_TEXTURE_2D) || (ti->texture.target == GL_TEXTURE_3D)) &&
        (ti->slot < UINT_MAX);
}

////////////////////////////////////////////////////////////////////////////////
bool bhDeviceGL::IsPipelineValid_World(bhResourceID pipelineID)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    return pipeline->programID > 0;
}

bhTextureInputGL bhDeviceGL::bhWorldMaterial_SetupTextureInput(const std::string* textureName, bhSamplerSlots slot)
{
    char* path = bhUtil_CreatePath(bhEnv_GetEnvString(ENV_TEXTURES_PATH), textureName->c_str());
    bhImage* img = bhImage_CreateFromFile(path, 4);

    if (img == nullptr)
    {
        bhLog_Message(LOG_TYPE_ERROR, "Could not load image %s", path);
        bhUtil_FreePath(&path);
        return {};
    }

    if (img->retCode == 1)
    {
        bhLog_Message(LOG_TYPE_WARNING, "Loading image %s: Runtime conversion", path);
    }
    bhUtil_FreePath(&path);
    bhResourceID newTexID = CreateTextureFromImage(img, textureName->c_str(), 0, 0);
    assert(newTexID != BH_INVALID_RESOURCE);

    bhTextureInputGL outTextureInput;
    outTextureInput.texture = *textureManager.GetResourceObject(newTexID);
    outTextureInput.slot = slot;
    return outTextureInput;
}

bhResourceID bhDeviceGL::CreateMaterial_World(bhResourceID pipelineID, const bhWorldMaterialCreateInfo* materialCI)
{
    bool result = true;
    bhMaterialGL_World* outMaterial = new bhMaterialGL_World();

    bhTextureInputGL* input_Albedo = &(outMaterial->textureInputs_v[SAMPLER_SLOT_ALBEDO]);
    *input_Albedo = bhWorldMaterial_SetupTextureInput(&(materialCI->texture_Albedo), SAMPLER_SLOT_ALBEDO);
    result &= bhWorldMaterial_IsTextureInputValid(input_Albedo);

    bhTextureInputGL* input_NS = &(outMaterial->textureInputs_v[SAMPLER_SLOT_NORMAL_SPECULAR]);
    *input_NS = bhWorldMaterial_SetupTextureInput(&(materialCI->texture_NormalSpecular), SAMPLER_SLOT_NORMAL_SPECULAR);
    result &= bhWorldMaterial_IsTextureInputValid(input_NS);

    return static_cast<bhResourceID>(outMaterial);
}

void bhDeviceGL::DestroyMaterial_World(bhResourceID materialID)
{
    bhMaterialGL_World* material = static_cast<bhMaterialGL_World*>(materialID);
    // Should reduce refcount on used textures etc.
}

void bhDeviceGL::UseMaterial_World(bhResourceID materialID)
{
    bhMaterialGL_World* mat = static_cast<bhMaterialGL_World*>(materialID);
    const bhTextureInputGL* input_Albedo = &(mat->textureInputs_v[SAMPLER_SLOT_ALBEDO]);
    glActiveTexture(GL_TEXTURE0 + (int)input_Albedo->slot);
    glBindTexture(input_Albedo->texture.target, input_Albedo->texture.id);

    const bhTextureInputGL* input_NormSpec = &(mat->textureInputs_v[SAMPLER_SLOT_NORMAL_SPECULAR]);
    glActiveTexture(GL_TEXTURE0 + (int)input_NormSpec->slot);
    glBindTexture(input_NormSpec->texture.target, input_NormSpec->texture.id);
}

////////////////////////////////////////////////////////////////////////////////
void bhDeviceGL::bhWorldPipeline_SetupVAO(bhPipelineGL_World* pipeline, int bindingsMask)
{
    glGenVertexArrays(1, &(pipeline->VAO));
    glBindVertexArray(pipeline->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, pipeline->vertexBuffer.buffer);

    if (bindingsMask & MESH_BINDING_BIT_POSITIONS)
    {
        glVertexAttribPointer((GLuint)MESH_BINDING_POSITIONS, 3, GL_FLOAT, GL_FALSE,
            sizeof(bhWorldVertex), (void*)offsetof(bhWorldVertex, bhWorldVertex::position));
        glEnableVertexAttribArray((GLuint)MESH_BINDING_POSITIONS);
    }
    if (bindingsMask & MESH_BINDING_BIT_NORMALS)
    {
        glVertexAttribPointer((GLuint)MESH_BINDING_NORMALS, 3, GL_FLOAT, GL_FALSE,
            sizeof(bhWorldVertex), (void*)offsetof(bhWorldVertex, bhWorldVertex::normal));
        glEnableVertexAttribArray((GLuint)MESH_BINDING_NORMALS);
    }
    if (bindingsMask & MESH_BINDING_BIT_TANGENTS)
    {
        glVertexAttribPointer((GLuint)MESH_BINDING_TANGENTS, 3, GL_FLOAT, GL_FALSE,
            sizeof(bhWorldVertex), (void*)offsetof(bhWorldVertex, bhWorldVertex::tangent));
        glEnableVertexAttribArray((GLuint)MESH_BINDING_TANGENTS);
    }
    if (bindingsMask & MESH_BINDING_BIT_UV_0)
    {
        glVertexAttribPointer((GLuint)MESH_BINDING_UV_0, 2, GL_FLOAT, GL_FALSE,
            sizeof(bhWorldVertex), (void*)offsetof(bhWorldVertex, bhWorldVertex::uv));
        glEnableVertexAttribArray((GLuint)MESH_BINDING_UV_0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pipeline->indexBuffer.buffer);

    //{
    //    glBindBuffer(GL_ARRAY_BUFFER, g_instanceDataBuffer.buffer);
    //    glVertexAttribPointer((GLuint)bhMeshInstanceBindings::INSTANCE_POSITIONS, 3, GL_FLOAT, GL_FALSE, 0, 0); // Matrix of 4x4 floats
    //    glEnableVertexAttribArray((GLuint)bhMeshInstanceBindings::INSTANCE_POSITIONS);
    //    glVertexAttribDivisor((GLuint)bhMeshInstanceBindings::INSTANCE_POSITIONS, 1);
    //}

    glBindVertexArray(GL_ZERO);
}

void bhDeviceGL::bhWorldPipeline_CreateDataBuffers(bhPipelineGL_World* pipeline)
{
    pipeline->vertexBuffer = CreateBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, sizeof(bhWorldVertex) * BH_BUFFER_MAX_VERTS); // TODO: Static_draw?
    glBindBuffer(GL_ARRAY_BUFFER, GL_ZERO); // TODO: Should CreateBuffer do this?

    pipeline->indexBuffer = CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, sizeof(bhMeshIdx_t) * BH_BUFFER_MAX_INDS); // TODO: Static_draw?
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_ZERO); // TODO: Should CreateBuffer do this?

    // Init instance transforms object
    //{
    //    g_instanceDataBuffer = bhGL_CreateBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, sizeof(bhVec3f) * BH_MAX_INSTANCES);
    //    glBindBufferBase(GL_ARRAY_BUFFER, bhShaderBindings::SHADER_BINDING_MODEL_TRANSFORM, g_instanceDataBuffer);
    //}

    uint8_t bindingsMask =
        MESH_BINDING_BIT_POSITIONS |
        MESH_BINDING_BIT_NORMALS |
        MESH_BINDING_BIT_TANGENTS |
        MESH_BINDING_BIT_UV_0;

    bhWorldPipeline_SetupVAO(pipeline, bindingsMask);
}

bhResourceID bhDeviceGL::CreatePipeline_World()
{
    std::vector<std::string> shaderNames_v { "World.vert","World.frag" };
    bhPipelineGL_World* outPipeline = new bhPipelineGL_World();

    outPipeline->programID = bhGL_CreateProgramFromShaders(&shaderNames_v);
    if (outPipeline->programID > 0)
    {
        // Init viewProjection object
        {
            glGenBuffers(1, &(outPipeline->viewDataBuffer));

            glBindBuffer(GL_UNIFORM_BUFFER, outPipeline->viewDataBuffer);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(bhCamera::ViewData), nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, bhShaderBindings::SHADER_BINDING_VIEW_PROJECTION, outPipeline->viewDataBuffer);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        // Init model object
        {
            glGenBuffers(1, &(outPipeline->modelBuffer));

            glBindBuffer(GL_UNIFORM_BUFFER, outPipeline->modelBuffer);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, bhShaderBindings::SHADER_BINDING_MODEL_TRANSFORM, outPipeline->modelBuffer);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        bhWorldPipeline_CreateDataBuffers(outPipeline);
    }
    return static_cast<bhResourceID>(outPipeline);
}

void bhDeviceGL::bhWorldPipeline_DestroyDataBuffers(bhPipelineGL_World* pipeline)
{
    // Destroy instance transforms object
    //bhGL_DestroyBuffer(&(pipeline->));

    glDeleteVertexArrays(1, &(pipeline->VAO));
    DestroyBuffer(&(pipeline->indexBuffer));
    DestroyBuffer(&(pipeline->vertexBuffer));
}

void bhDeviceGL::DestroyPipeline_World(bhResourceID pipelineID)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    bhWorldPipeline_DestroyDataBuffers(pipeline);
    // Destroy viewProjection object
    glDeleteBuffers(1, &(pipeline->viewDataBuffer));

    glUseProgram(0);
    glDeleteProgram(pipeline->programID);
    pipeline->programID = 0;
}

void bhDeviceGL::UsePipeline_World(bhResourceID pipelineID)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    assert(pipeline->programID > 0);
    glUseProgram(pipeline->programID);
}

void bhDeviceGL::SetCameraView_World(bhResourceID pipelineID, const bhCamera::ViewData* viewProjection)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    glNamedBufferSubData(pipeline->viewDataBuffer, 0, sizeof(bhCamera::ViewData), viewProjection);
}

void bhDeviceGL::UploadMesh_World(bhResourceID pipelineID, bhResourceID meshID)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    bhMesh* mesh = static_cast<bhMesh*>(meshID);

    GLsizei reqSize = mesh->numVerts * sizeof(bhWorldVertex);
    CopyDataToBuffer(&(pipeline->vertexBuffer), pipeline->vertexBufferWriteOffset, reqSize, mesh->verts);
    pipeline->vertexBufferWriteOffset += reqSize;
    mesh->offsets.offsetsGL.baseVertex = pipeline->baseVertex;
    pipeline->baseVertex += mesh->numVerts;

    reqSize = mesh->numInds * sizeof(bhMeshIdx_t);
    CopyDataToBuffer(&(pipeline->indexBuffer), pipeline->indexBufferWriteOffset, reqSize, mesh->inds);
    pipeline->indexBufferWriteOffset += reqSize;
    mesh->offsets.offsetsGL.baseIndex = pipeline->baseIndex;
    pipeline->baseIndex += mesh->numInds;
}

void bhDeviceGL::RenderMesh_World(bhResourceID pipelineID, bhResourceID meshID, const glm::mat4* transform)
{
    bhPipelineGL_World* pipeline = static_cast<bhPipelineGL_World*>(pipelineID);
    glNamedBufferSubData(pipeline->modelBuffer, 0, sizeof(glm::mat4), transform);

    glBindVertexArray(pipeline->VAO);
    bhMesh* mesh = static_cast<bhMesh*>(meshID);
    glDrawRangeElementsBaseVertex(GL_TRIANGLES, mesh->offsets.offsetsGL.baseIndex, mesh->offsets.offsetsGL.baseIndex + mesh->numInds, mesh->numInds,
        GL_UNSIGNED_INT, nullptr, mesh->offsets.offsetsGL.baseVertex);
}

//void bhWorldPipeline_UploadInstanceData(const bhPipelineGL_World* pipeline, const bhModelData* modelData, size_t numInstances)
//{
//    uint32_t numRenderInstances = bhMath_Min((uint32_t)BH_MAX_INSTANCES, (uint32_t)numInstances);
//    // TODO : Bounds check
//    glNamedBufferSubData(g_instanceDataBuffer.buffer, 0, sizeof(bhVec3f) * numRenderInstances, modelData);
//}
