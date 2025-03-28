#include <list>
#include "bhEvent.hpp"

namespace bhEvent
{
	static std::list<Event> eventQueue;

	void PushEvent(const Event& evt)
	{
		eventQueue.push_back(evt);
	}

	bool GetNextEvent(Event& evt)
	{
		if (eventQueue.empty())
		{
			return false;
		}
		evt = eventQueue.front();
		eventQueue.pop_front();
		return true;
	}
}
