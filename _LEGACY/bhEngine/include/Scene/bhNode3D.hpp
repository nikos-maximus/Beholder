#ifndef BH_NODE3D_HPP
#define BH_NODE3D_HPP

#include <glm/mat4x4.hpp>

class bhNode3D
{
public:
  const glm::mat4& GetTransform() { return transform; }
  const glm::mat4& GetTransform() const { return transform; }
  void SetPosition(const glm::vec3& t);
  void SetPosition(float x, float y, float z);
  void Move(const glm::vec3& t);
  glm::vec3 GetPosition() const;
  void Rotate(const glm::vec3& axis, float degrees);

protected:
  bhNode3D() = default;
  inline void SetTransform(const glm::mat4& t) { transform = t; }

private:
  glm::mat4 transform{ 1.0f }; //TODO: Init to Identoty redundant?
};

#endif //BH_NODE3D_HPP
