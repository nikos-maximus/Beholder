#ifndef BH_EVENTID_HPP
#define BH_EVENTID_HPP

namespace bhEvent
{
	enum class Type
	{
		EVT_NONE,
		EVT_QUIT,
		EVT_ESCAPE,
		EVT_RESET,
		EVT_UNDO,
		EVT_START,
		EVT_RESUME,
		EVT_HIGHSCORE,

		EVT_FWD,
		EVT_BACK,
		EVT_LEFT,
		EVT_RIGHT,
		EVT_TURN_LEFT,
		EVT_TURN_RIGHT,
		EVT_UP,
		EVT_DOWN,
		EVT_LBUTTON,
		EVT_MBUTTON,
		EVT_RBUTTON,
		EVT_MOUSE_MOVE,

		EVT_TOGGLE_TEXTURE,
		EVT_TOGGLE_WIREFRAME,
		EVT_TOGGLE_LIGHTING,
		EVT_TOGGLE_NORMALS,

		EVT_TOGGLE_EDITOR,

		EVT_COUNT
	};

	struct Event
	{
		Type type;

		struct MouseMotion
		{
			float x, y, xrel, yrel;
		};

		union Data
		{
			MouseMotion mouseMotion;
		};

		Data data;
	};

	void PushEvent(const Event& evt);
	bool GetNextEvent(Event& evt);
}

#endif //BH_EVENTID_HPP
