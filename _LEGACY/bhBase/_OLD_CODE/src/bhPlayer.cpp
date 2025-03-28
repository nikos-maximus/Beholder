#include "bhPlayer.hpp"

////////////////////////////////////////////////////////////////////////////////
void bhPlayer::Move(glm::vec3 const& inputAxis)
{
    static constexpr float speedMod = 0.2f;
    glm::vec3 pos = bhTransform_GetTranslation(transform);
    pos += *(camera.GetFwdVec()) * inputAxis.x * speedMod;
    pos += *(camera.GetRightVec()) * inputAxis.y * speedMod;
    bhTransform_SetTranslation(transform, pos);
}

void bhPlayer::UpdateCamera(glm::vec2 const& axisMove)
{
    camera.Yaw(axisMove.x);
    camera.Pitch(axisMove.y);
    glm::vec3 pos = bhTransform_GetTranslation(transform);
    camera.UpdateView(&pos);
}
