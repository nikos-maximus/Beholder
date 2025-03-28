#pragma once
#include <glm/vec3.hpp>

template<typename Scalar_T>
class bhBox
{
public:
	bhBox(Scalar_T _size)
		: extent(_size / Scalar_T(2))
	{}

	bhBox(glm::vec<3, Scalar_T> const& _size)
		: extent(_size * glm::vec<3, Scalar_T>(0.5))
	{}

	inline glm::vec<3, Scalar_T> const& GetPosition() const
	{ 
		return center; 
	}

	inline void SetPosition(glm::vec<3, Scalar_T> const& _pos) { center = _pos; }
	inline void SetPosition(Scalar_T x, Scalar_T y, Scalar_T z) { center.x = x; center.y = y; center.z = z; }

	inline glm::vec<3, Scalar_T> const& GetExtent() const 
	{ 
		return extent; 
	}

	inline glm::vec<3, Scalar_T> GetSize() const 
	{ 
		return extent + extent; 
	}

	inline void SetSize(glm::vec<3, Scalar_T> const& siz) { extent = siz * Scalar_T(0.5); }
	inline void SetSize(Scalar_T sizx, Scalar_T sizy, Scalar_T sizz) { SetSize(glm::vec<3, Scalar_T>(sizx, sizy, sizz)); }

	inline bool Contains(glm::vec<3, Scalar_T> const& point) const
	{
		return
			(point.x >= (center.x - extent.x)) && (point.x <= (center.x + extent.x)) &&
			(point.y >= (center.y - extent.y)) && (point.y <= (center.y + extent.y)) &&
			(point.z >= (center.z - extent.z)) && (point.z <= (center.z + extent.z));
	}

	inline bool Contains(bhBox const& box) const
	{
		return Contains(box.center - box.extent) && Contains(box.center + box.extent);
	}

	inline bool Overlaps(bhBox const& box) const
	{
		return !DoesNotOverlap(box);
	}

	inline bool DoesNotOverlap(bhBox const& box) const
	{
		return
			(center.x - extent.x) > (box.center.x + box.extent.x) ||
			(center.x + extent.x) < (box.center.x - box.extent.x) ||

			(center.y - extent.y) > (box.center.y + box.extent.y) ||
			(center.y + extent.y) < (box.center.y - box.extent.y) ||

			(center.z - extent.z) > (box.center.z + box.extent.z) ||
			(center.z + extent.z) < (box.center.z - box.extent.z);
	}

protected:
	glm::vec<3, Scalar_T> center;
	glm::vec<3, Scalar_T> extent;

private:
};

typedef bhBox<float> bhBoxf;
