#ifndef BH_PLAYER_HPP
#define BH_PLAYER_HPP

#include "bhTransform.hpp"
#include "bhCamera.hpp"

////////////////////////////////////////////////////////////////////////////////
class bhPlayer
{
public:
    inline glm::vec3 GetPosition() const
    {
        return bhTransform_GetTranslation(transform);
    }

    inline void SetPosition(const glm::vec3* t)
    { 
        bhTransform_SetTranslation(transform, *t);
    }
    
    inline void SetPosition(float x, float y, float z)
    {
        bhTransform_SetTranslation(transform, x, y, z);
    }

    void Move(glm::vec3 const& inputAxis);
    void UpdateCamera(glm::vec2 const& axisMove);
    
    bhCamera& GetCamera()
    {
        return camera;
    }

protected:
    glm::mat4 transform;
    bhCamera camera;

private:
};

#endif //BH_PLAYER_HPP
