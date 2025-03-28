#pragma once
#include <vector>
#include "Math/bhVector.h"

namespace bhPhysics
{
    bool Create();
    void Destroy();
    void StepSimulation(float dTime);
    void ClearWorld();
    void SetGravity(bhVec3f const& g);
    void AddPlayerCapsule(bhVec3f const& origin);// , void const* camera_p);
    void MovePlayer(bhVec3f const& vel);
    bhVec3f GetPlayerPosition();
    void AddStaticBody(char const* name, bhVec3f const& location);
    void AddStaticBodies(char const* name, std::vector<bhVec3f> const& locations, size_t startIndex, size_t numInds);
    void CreateCollisionShape_Box(char const* name, bhVec3f const& size);
    void DeleteCollisionShape(char const* name);
}
