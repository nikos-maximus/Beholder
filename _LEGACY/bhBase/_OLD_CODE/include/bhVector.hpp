#ifndef BH_VECTOR_H
#define BH_VECTOR_H

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
template <typename Element_t>
class bhFixed_Vector
{
public:
    bhFixed_Vector(size_t _num_elements)
        :num_elements(_num_elements)
    {
        elements = calloc(num_elements, sizeof(Element_t));
    }

    ~bhFixed_Vector()
    {
        delete(elements);
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

protected:
private:
    bhFixed_Vector(); // Don't allow empty vectors

    Element_t* elements = nullptr;
    size_t num_elements = 0;
};

#endif
