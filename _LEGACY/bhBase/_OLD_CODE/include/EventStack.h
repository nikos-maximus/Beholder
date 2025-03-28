#ifndef BH_EVENTSTACK_H
#define BH_EVENTSTACK_H

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

template<class T> class EventStack
{
public:

	EventStack(unsigned short numEvents)
		:baseSize(numEvents)
		,size(0)
	{
		events = new T[baseSize];
	}

	~EventStack()
	{
		delete[] events;
	}

	inline void Push(T evt)
	{
		assert(size < baseSize);
		events[size++] = evt;
	}

	inline void Clear()
	{
		size = 0;
	}

	inline T Pop()
	{
		return events[--size];
	}

	inline bool Empty()
	{
		return (size == 0);
	}

protected:
private:

	short baseSize,size;
	T* events;
};

////////////////////////////////////////////////////////////////////////////////

#endif
