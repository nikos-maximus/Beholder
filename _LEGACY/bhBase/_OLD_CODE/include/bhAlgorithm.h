#pragma once
#include <inttypes.h>

namespace bhAlgorithm
{
	template<typename S, typename T>
	size_t BinSearch(S* searchEl, T* elements, size_t numElements, int(*ElemCmpFun)(void const* first, void const* second))
	{
		if (numElements > 0)
		{
			size_t start = 0, end = numElements; //end is one-past-the-last valid element
			size_t mid = 0;
			do
			{
				mid = (start + end) / 2;
				if(ElemCmpFun(searchEl, &elements[mid]) == 0)
				{
					return mid;
				}
				if (ElemCmpFun(searchEl, &elements[mid]) < 0)
				{
					end = mid;
					continue;
				}
				if (ElemCmpFun(searchEl, &elements[mid]) > 0)
				{
					start = mid;
					continue;
				}
			} while (end > start);
		}
		return 0;
	}

	char const* RFind(char const* string, char c);
}
