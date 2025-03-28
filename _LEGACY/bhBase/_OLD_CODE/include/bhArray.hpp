#ifndef BH_ARRAY_H
#define BH_ARRAY_H

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
template<class Element_t, size_t num_elements>
class bhArray
{
public:
	bhArray() {}

	bhArray(Element_t const& value)
	{
		for (size_t e = 0; e < num_elements; ++e)
		{
			elements[e] = value;
		}
	}

	inline Element_t& operator[](size_t index)
	{
		assert(index < num_elements);
		return elements[index];
	}

	inline Element_t const& operator[](size_t index) const
	{
		assert(index < num_elements);
		return elements[index];
	}

	inline size_t Size() const
	{
		return num_elements;
	}

	bhArray<Element_t, num_elements> Clone()
	{
		bhArray<Element_t, num_elements> newArray;
		for (size_t e = 0; e < num_elements; ++e)
		{
			newArray.elements[e] = elements[e];
		}
		return newArray;
	}

protected:
private:
	Element_t elements[num_elements];
};

#endif //BH_ARRAY_H
