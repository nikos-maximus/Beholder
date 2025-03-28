#ifndef BH_ARRAY_HPP
#define BH_ARRAY_HPP

#include <assert.h>

template<typename Item_t, size_t _siz>
class bhArray
{
public:
	Item_t& operator[](size_t _idx)
	{
		assert(_idx < _siz);
		return _items[_idx];
	}

	const Item_t& operator[](size_t _idx) const
	{
		assert(_idx < _siz);
		return _items[_idx];
	}

	size_t Size() const { return _siz; }

protected:
private:
	Item_t _items[_siz];
};

#endif //BH_ARRAY_HPP
