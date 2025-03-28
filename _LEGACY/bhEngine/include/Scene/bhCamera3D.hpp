#ifndef BH_CAMERA_3D_HPP
#define BH_CAMERA_3D_HPP

#include "Scene/bhNode3D.hpp"
#include "bhCamera.hpp"

class bhCamera3D : public bhNode3D
{
public:
  void LookAt(const glm::vec3& target);
  void LookAt(float x, float y, float z);
  inline const glm::mat4& GetProjection() { return camera.GetProjection(); }

protected:
private:
  bhCamera camera;
};

#endif //BH_CAMERA_3D_HPP
