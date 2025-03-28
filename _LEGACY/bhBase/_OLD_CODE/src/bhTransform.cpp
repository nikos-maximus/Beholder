#include "bhTransform.hpp"
#include <glm/ext.hpp>

void bhTransform_SetTranslation(glm::mat4& m, const glm::vec3& t)
{
    m[3][0] = t.x;
    m[3][1] = t.y;
    m[3][2] = t.z;
}

void bhTransform_SetTranslation(glm::mat4& m, float x, float y, float z)
{
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
}

void bhTransform_AddTranslation(glm::mat4& m, const glm::vec3& t)
{
    m[3][0] += t.x;
    m[3][1] += t.y;
    m[3][2] += t.z;
}

void bhTransform_AddTranslation(glm::mat4& m, float x, float y, float z)
{
    m[3][0] += x;
    m[3][1] += y;
    m[3][2] += z;
}

void bhTransform_SetScale(glm::mat4& m, const glm::vec3& t)
{
    m[0][0] *= t.x;
    m[1][1] *= t.y;
    m[2][2] *= t.z;
}

void bhTransform_SetScale(glm::mat4& m, float x, float y, float z)
{
    m[0][0] *= x;
    m[1][1] *= y;
    m[2][2] *= z;
}

void bhTransform_Rotate(glm::mat4& m, const glm::vec3& axis, float radians)
{
    m = glm::rotate(m, radians, axis);
}

glm::vec3 bhTransform_GetTranslation(const glm::mat4& m)
{
    return glm::vec3(m[3][0], m[3][1], m[3][2]);
}
