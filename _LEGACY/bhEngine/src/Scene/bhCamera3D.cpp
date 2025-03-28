#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "Scene/bhCamera3D.hpp"

void bhCamera3D::LookAt(const glm::vec3& target)
{
	camera.fwd = glm::normalize(target - GetPosition());
	camera.right = glm::normalize(glm::cross(camera.fwd, camera.up));

	SetTransform(glm::lookAt(GetPosition(), target, camera.up));
}

void bhCamera3D::LookAt(float x, float y, float z)
{
	LookAt(glm::vec3(x, y, z));
}
