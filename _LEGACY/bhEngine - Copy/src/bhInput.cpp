#include <SDL3/SDL.h>
#include "bhInput.hpp"
#include "bhConfig.hpp"
//#include "bhEvent.hpp"
#include "bhSystem.hpp"

namespace bhInput
{
	////////////////////////////////////////////////////////////////////////////////
	static double mousePosX = 0.0, mousePosY = 0.0;
	static double mouseMoveX = 0.0f, mouseMoveY = 0.0f;
	static SDL_bool inputEnabled = SDL_FALSE;
	int isQuitting = 0;

	////////////////////////////////////////////////////////////////////////////////
	struct KeyBinding
	{
		Uint16 key{ SDL_SCANCODE_UNKNOWN };
		Uint8 prevState{ 0 };
		bhEvent::Type evtType{ bhEvent::Type::EVT_NONE };
		PostEventCondition eventCondition{ PostEventCondition::EC_HOLD };
	};

	static KeyBinding g_keyBindings[CMD_NUM_COMMANDS];

	////////////////////////////////////////////////////////////////////////////////
	CmdType FindCommandForKey(int key)
	{
		for (int cmd = 0; cmd < CMD_NUM_COMMANDS; ++cmd)
		{
			if (g_keyBindings[cmd].key == key)
			{
				return static_cast<CmdType>(cmd);
			}
		}
		return CMD_NUM_COMMANDS;
	}

	void RegisterKeyBinding(int key, CmdType cmd, bhEvent::Type evtType, PostEventCondition cond)
	{
		g_keyBindings[cmd].key = key;
		g_keyBindings[cmd].evtType = evtType;
		g_keyBindings[cmd].eventCondition = cond;
	}

	void UnregisterKeyBinding(int key)
	{
		CmdType cmd = FindCommandForKey(key);
		if (cmd < CMD_NUM_COMMANDS)
		{
			g_keyBindings[cmd].key = SDL_SCANCODE_UNKNOWN;
		}
	}

	void RegisterDefaultKeyBindings()
	{
		RegisterKeyBinding(SDL_SCANCODE_ESCAPE, CMD_ESCAPE, bhEvent::Type::EVT_ESCAPE, PostEventCondition::EC_PRESS);

		RegisterKeyBinding(SDL_SCANCODE_W, CMD_FWD, bhEvent::Type::EVT_FWD, PostEventCondition::EC_HOLD);
		RegisterKeyBinding(SDL_SCANCODE_S, CMD_BACK, bhEvent::Type::EVT_BACK, PostEventCondition::EC_HOLD);
		RegisterKeyBinding(SDL_SCANCODE_A, CMD_LEFT, bhEvent::Type::EVT_LEFT, PostEventCondition::EC_HOLD);
		RegisterKeyBinding(SDL_SCANCODE_D, CMD_RIGHT, bhEvent::Type::EVT_RIGHT, PostEventCondition::EC_HOLD);
		RegisterKeyBinding(SDL_SCANCODE_Q, CMD_TURN_LEFT, bhEvent::Type::EVT_TURN_LEFT, PostEventCondition::EC_HOLD);
		RegisterKeyBinding(SDL_SCANCODE_E, CMD_TURN_RIGHT, bhEvent::Type::EVT_TURN_RIGHT, PostEventCondition::EC_HOLD);
	}

	void HandleKeyState()
	{
		int numKeys = 0;
		const Uint8* keys = SDL_GetKeyboardState(&numKeys);
		for (int cmd = 0; cmd < CMD_NUM_COMMANDS; ++cmd)
		{
			KeyBinding* kb = &(g_keyBindings[cmd]);
			if (keys[kb->key])
			{
				if ((kb->eventCondition == PostEventCondition::EC_HOLD) ||
					((kb->eventCondition == PostEventCondition::EC_PRESS) && !kb->prevState))
				{
					bhEvent::Event evt = {};
					evt.type = kb->evtType;
					PushEvent(evt);
				}
			}
			else
			{
				if ((kb->eventCondition == PostEventCondition::EC_RELEASE) && kb->prevState)
				{
					bhEvent::Event evt = {};
					evt.type = kb->evtType;
					PushEvent(evt);
				}
			}
			kb->prevState = keys[kb->key];
		}
	}

	void GetMouseMove(float& moveX, float& moveY)
	{
		const bhConfig::InputSettings& is = bhSystem::Config()->inputSt;
		moveX = (float)mouseMoveX * is.mouse_sensitivity;
		moveY = (float)mouseMoveY * is.mouse_sensitivity;
	}

	void Enable()
	{
		inputEnabled = SDL_TRUE;
	}

	void Disable()
	{
		inputEnabled = SDL_FALSE;
	}

	void ProcessEvent(const SDL_Event* evt)
	{
		switch (evt->type)
		{
			case SDL_EVENT_MOUSE_MOTION:
			{
				bhEvent::Event bhEvt = { bhEvent::Type::EVT_MOUSE_MOVE };
				bhEvt.data.mouseMotion.x = evt->motion.x;
				bhEvt.data.mouseMotion.y = evt->motion.y;
				bhEvt.data.mouseMotion.xrel = evt->motion.xrel;
				bhEvt.data.mouseMotion.yrel = evt->motion.yrel;
				
				bhEvent::PushEvent(bhEvt);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	void HandleSDLEvents()
	{
		SDL_PumpEvents();
		//g_inputAxis.x = g_inputAxis.y = 0.0f;
		mouseMoveX = mouseMoveY = 0.0f;

		static SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			ProcessEvent(&evt);
		}
		HandleKeyState();
	}

	void SetMouseMode_Cursor(SDL_Window* wnd)
	{
		SDL_SetWindowGrab(wnd, SDL_FALSE);
		//SDL_ShowCursor(SDL_ENABLE);
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	void SetMouseMode_Look(SDL_Window* wnd)
	{
		SDL_SetWindowGrab(wnd, SDL_TRUE);
		//SDL_ShowCursor(SDL_DISABLE);
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
}
