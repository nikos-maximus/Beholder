#pragma once

template <class T, size_t capacity> class bhQueue
{
public:

	bhQueue()
		: first(0)
		, numItems(0)
	{}

	inline size_t GetCapacity() const { return capacity; }
	inline size_t GetSize() const { return size_t(numItems); }
	inline bool IsEmpty() { return numItems <= 0; } // although less than should never happen..
	inline bool IsFull() { return numItems >= capacity; }// although greater than should never happen..
	
	inline bool Enqueue(T const& item)
	{
		if (IsFull())
		{
			return false;
		}
		items[(first + numItems) % capacity] = item;
		++numItems;
		return true;
	}

	inline bool DequeueNewest(T* item)
	{
		if (IsEmpty())
		{
			return false;
		}
		*item = items[(first + --numItems) % capacity];
		return true;
	}

	inline bool DequeueOldest(T* item)
	{
		if (IsEmpty())
		{
			return false;
		}
		*item = items[first];
		first = ++first % capacity;
		--numItems;
		return true;
	}

protected:
private:

	T items[capacity];
	int first, numItems;
};
