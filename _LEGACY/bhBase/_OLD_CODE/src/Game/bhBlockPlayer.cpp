#define _USE_MATH_DEFINES
#include "Game/bhBlockPlayer.hpp"

////////////////////////////////////////////////////////////////////////////////
static constexpr float TRANSITION_TIME = 0.55f;
static constexpr float BOB_HEIGHT = 0.1f;

bhBlockPlayer::Heading bhBlockPlayer::DetermineHeading() const
{
    const glm::vec3* fwv = camera.GetFwdVec();
    if (glm::abs(fwv->x) > glm::abs(fwv->y))
    {
        return fwv->x > 0.f ? Heading::EAST : Heading::WEST;
    }
    // else
    return fwv->y > 0.f ? Heading::NORTH : Heading::SOUTH;
}

void bhBlockPlayer::Tick(bhTime_t deltaTime)
{
    if (moveInterp.IsPlaying())
    {
        moveInterp.Tick(deltaTime);
        glm::vec3 newPos = moveInterp.GetValue();
        static float origHeight = newPos.z;
        newPos.z = origHeight + sinf(M_PI * moveInterp.GetTime()) * BOB_HEIGHT;
        bhTransform_SetTranslation(transform, newPos);
        return;
    }
    if (turnInterp.IsPlaying())
    {
        turnInterp.Tick(deltaTime);
        glm::vec3 newFwd = bhTransform_GetTranslation(transform) + turnInterp.GetValue();
        camera.LookAt(&newFwd);
        return;
    }
}

void bhBlockPlayer::StartMove(const glm::vec3* endPoint)
{
    glm::vec3 pos = GetPosition();
    moveInterp.SetStart(&pos);
    moveInterp.SetEnd(endPoint);
    moveInterp.Start(TRANSITION_TIME);
}

void bhBlockPlayer::StartTurn(bhCommandID cmdID)
{
    turnInterp.SetStart(camera.GetFwdVec());
    glm::vec3 camRight = *camera.GetRightVec();
    //int hd = (int)DetermineHeading();
    switch (cmdID)
    {
        case CMD_ID_LTURN:
        {
            glm::vec3 camLeft = -(*camera.GetRightVec());
            turnInterp.SetEnd(&camLeft);
            break;
        }
        case CMD_ID_RTURN:
        {
            turnInterp.SetEnd(camera.GetRightVec());
            break;
        }
    }
    turnInterp.Start(TRANSITION_TIME);
}
