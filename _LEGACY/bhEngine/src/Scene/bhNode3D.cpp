#include "Scene/bhNode3D.hpp"
#include <glm/ext/matrix_transform.hpp>

void bhNode3D::SetPosition(const glm::vec3& t)
{
  //glm::translate(transform, t);
  transform[3].x = t.x;
  transform[3].y = t.y;
  transform[3].z = t.z;
}

void bhNode3D::SetPosition(float x, float y, float z)
{
  //glm::translate(transform, glm::vec3(x, y, z));
  transform[3].x = x;
  transform[3].y = y;
  transform[3].z = z;
}

void bhNode3D::Move(const glm::vec3& t)
{
  //glm::translate(transform, t);
  transform[3].x += t.x;
  transform[3].y += t.y;
  transform[3].z += t.z;
}

glm::vec3 bhNode3D::GetPosition() const
{
  return { transform[3].x, transform[3].y, transform[3].z };
}

void bhNode3D::Rotate(const glm::vec3& axis, float degrees)
{
  transform = glm::rotate(transform, glm::radians(degrees), axis);
}
