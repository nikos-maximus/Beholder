#ifndef BH_CAMERA_HPP
#define BH_CAMERA_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class bhCamera
{
  friend class bhCamera3D;

public:
  bhCamera();
  void SetProjection(float fovy, float aspect, float near, float far);
  void Yaw(float degrees);
  void Pitch(float degrees);
  void Roll(float degrees);

  const glm::vec3& GetFwdVec() const
  {
    return fwd;
  }

  const glm::vec3& GetRightVec() const
  {
    return right;
  }

  //const glm::mat4* GetView() const 
  //{ 
  //    return viewData.view; 
  //}

  //const glm::mat4* GetProjection() const 
  //{ 
  //    return viewData.projection; 
  //}

  const glm::mat4& GetProjection();

  static const float DEFAULT_FOV_Y_DEGS; // In DEGREES
  static const glm::vec3 WORLD_UP_VEC;

protected:
  glm::mat4 projection{ 1.0f };
  glm::vec3 fwd, right, up;

private:
};

#endif //BH_CAMERA_HPP
