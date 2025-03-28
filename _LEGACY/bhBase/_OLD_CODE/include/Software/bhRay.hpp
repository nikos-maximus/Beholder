#ifndef BH_RAY_HPP
#define BH_RAY_HPP

#include <glm/vec3.hpp>

class bhRay
{
public:
	bhRay() = default;
	bhRay(const glm::vec3& _origin, const glm::vec3& _direction)
		: origin(_origin)
		, direction(_direction)
	{}

	__forceinline glm::vec3 PointAtParam(float t)
	{
		return origin + direction * t;
	}

	const glm::vec3& GetDirection() const
	{
		return direction;
	}

protected:
private:
	glm::vec3 origin{ 0.f };
	glm::vec3 direction{ 0.f };
};

#endif //BH_RAY_HPP
