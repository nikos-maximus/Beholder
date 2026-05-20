#ifndef BH_CAMERA_HPP
#define BH_CAMERA_HPP

#include <glm/mat4x4.hpp>

class bhCamera
{
public:
	struct ViewProjection
	{
		glm::mat4 view, projection;
	};

	inline void SetAspect(float _aspect) { aspect = _aspect; }
	inline const ViewProjection& GetViewProjection() const { return viewProj; }

protected:
private:
	ViewProjection viewProj;
	glm::vec3 position;
	float aspect{ 1.0f };
};

#endif //BH_CAMERA_HPP
