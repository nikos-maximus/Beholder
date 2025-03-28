#ifndef BH_INPUT_H
#define BH_INPUT_H

#include "bhEvent.hpp"

union SDL_Event;
struct SDL_Window;

namespace bhInput
{
	////////////////////////////////////////////////////////////////////////////////
	enum CmdType
	{
		CMD_NONE,
		CMD_QUIT,
		CMD_ESCAPE,
		//CMD_OPEN_EDITOR,

		CMD_FWD,
		CMD_BACK,
		CMD_LEFT,
		CMD_RIGHT,
		CMD_TURN_LEFT,
		CMD_TURN_RIGHT,

		CMD_TOGGLE_UI_OVERLAY,

		CMD_NUM_COMMANDS
	};

	enum PostEventCondition
	{
		EC_HOLD,
		EC_PRESS,
		EC_RELEASE,

		NUM_EVENT_CONDITIONS
	};

	////////////////////////////////////////////////////////////////////////////////
	void Enable();
	void Disable();
	void HandleSDLEvents(bool(*UIEventHandler)(const SDL_Event*));

	void RegisterKeyBinding(int key, CmdType cmd, bhEvent::Type evtType, PostEventCondition cond);
	void UnregisterKeyBinding(int key);
	void RegisterDefaultKeyBindings();
}

#endif //BH_INPUT_H
