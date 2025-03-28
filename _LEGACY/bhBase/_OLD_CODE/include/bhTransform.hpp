#ifndef BH_TRANSFORM_HPP
#define BH_TRANSFORM_HPP

#include <glm/mat4x4.hpp>

void bhTransform_SetTranslation(glm::mat4& m, const glm::vec3& t);
void bhTransform_SetTranslation(glm::mat4& m, float x, float y, float z);
void bhTransform_AddTranslation(glm::mat4& m, const glm::vec3& t);
void bhTransform_AddTranslation(glm::mat4& m, float x, float y, float z);
void bhTransform_Rotate(glm::mat4& m, const glm::vec3& axis, float radians);
void bhTransform_SetScale(glm::mat4& m, const glm::vec3& t);
void bhTransform_SetScale(glm::mat4& m, float x, float y, float z);
glm::vec3 bhTransform_GetTranslation(const glm::mat4& m);

#endif //BH_TRANSFORM_HPP
