#ifndef BH_BLOCK_PLAYER_HPP
#define BH_BLOCK_PLAYER_HPP

#include "bhPlayer.hpp"
#include "bhInput.hpp"
#include "Math/bhInterp.hpp"
#include "bhDefines.h"

////////////////////////////////////////////////////////////////////////////////
class bhBlockPlayer : public bhPlayer
{
public:
    enum class Heading
    {
        EAST,
        NORTH,
        WEST,
        SOUTH,

        NUM_HEADINGS
    };

    bhBlockPlayer() = default;

    bool IsResting() const
    {
        return !moveInterp.IsPlaying() && !turnInterp.IsPlaying();
    }

    void Tick(bhTime_t deltaTime);

    void StartMove(const glm::vec3* endPoint);

    void StartTurn(bhCommandID cmdID);

    Heading DetermineHeading() const;

protected:
    bhInterp<glm::vec3, bhTime_t, bhLinear> moveInterp;
    bhInterp<glm::vec3, bhTime_t, bhLinear> turnInterp;

private:
};

#endif //BH_BLOCK_PLAYER_HPP
