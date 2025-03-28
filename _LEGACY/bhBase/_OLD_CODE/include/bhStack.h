#pragma once
#include <assert.h>
#include "bhDefs.h"

template<class T, int32_t CAPACITY> class bhStack
{
public:

	bhStack()
		: head(-1)
		, MAX_INDEX(CAPACITY - 1)
	{}

	bool IsEmpty()
	{
		return head < 0;
	}

	bool IsFull()
	{
		return head == MAX_INDEX;
	}

	void Push(T item)
	{
		assert(head < MAX_INDEX);
		items[++head] = item;
	}

	T Pop()
	{
		assert(head >= 0);
		return items[head--];
	}

protected:
private:

	T items[CAPACITY];
	const int32_t MAX_INDEX;
	int32_t head;
};
