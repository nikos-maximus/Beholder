#ifndef BH_INPUT_H
#define BH_INPUT_H

#include <SDL.h>
#include "bhEvent.hpp"

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

		CMD_TOGGLE_EDITOR,

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
	void HandleSDLEvents();

	void ProcessEvent(const SDL_Event* evt);
	void HandleKeyState();

	void SetMouseMode_Cursor(SDL_Window* wnd);
	void SetMouseMode_Look(SDL_Window* wnd);

	void RegisterKeyBinding(int key, CmdType cmd, bhEvent::Type evtType, PostEventCondition cond);
	void UnregisterKeyBinding(int key);
	void RegisterDefaultKeyBindings();
}

#endif //BH_INPUT_H
