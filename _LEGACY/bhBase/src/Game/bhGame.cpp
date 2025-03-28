#include "Game/bhGame.hpp"
#include "bhInput.hpp"
#include "bhSystem.hpp"

////////////////////////////////////////////////////////////////////////////////
bool bhGame::Init(char* argv[])
{
	if (!bhSystem::Get().Init(argv))
	{
		return false;
	}
	//bhAudio::Play();
	//bhInput::RegisterKeyBinding(SDL_SCANCODE_ESCAPE, bhInput::CMD_ESCAPE, bhEvent::Type::EVT_ESCAPE, bhInput::EC_PRESS);
	bhInput::RegisterKeyBinding(SDL_SCANCODE_DELETE, bhInput::CMD_QUIT, bhEvent::Type::EVT_QUIT, bhInput::EC_PRESS);
	bhInput::RegisterKeyBinding(SDL_SCANCODE_INSERT, bhInput::CMD_TOGGLE_EDITOR, bhEvent::Type::EVT_TOGGLE_EDITOR, bhInput::EC_PRESS);
	bhInput::RegisterDefaultKeyBindings();
	//bhInput::SetMouseLook(true);

	world.Init();
	//world.Load_Resource("Map01.bhm");
	//bhShowWindow(bhSystem::Get().MainWindow());

	auto rd = bhSystem::Get().RenderDevice();
	if (!rd->InitImGui(bhSystem::Get().MainWindow()))
	{
		return false;
	}
	editor.reset(new bhEditor(world));

	bhInput::SetMouseMode_Look(bhSystem::Get().MainWindow());

	return true;
}

void bhGame::Destroy()
{
	if (editor)
	{
		auto rd = bhSystem::Get().RenderDevice();
		rd->DestroyImGui();
		editor.reset();
	}

	//bhInput::Disable();
	world.Destroy();

	bhSystem::Get().Destroy();
}

void bhGame::EnterMode_Menu()
{
	state = bhGameState::MENU;
	bhInput::SetMouseMode_Cursor(bhSystem::Get().MainWindow());
}

void bhGame::EnterMode_Game()
{
	state = bhGameState::RUN;
	bhInput::SetMouseMode_Look(bhSystem::Get().MainWindow());
}

void bhGame::EnterMode_Editor()
{
	state = bhGameState::EDITOR;
	bhInput::SetMouseMode_Cursor(bhSystem::Get().MainWindow());
}

void bhGame::HandleBhEvents()
{
	static bhEvent::Event evt;
	while (bhEvent::GetNextEvent(evt))
	{
		switch (evt.type)
		{
			case bhEvent::Type::EVT_ESCAPE:
			{
				switch (state)
				{
					case bhGameState::RUN:
					{
						EnterMode_Menu();
						break;
					}
					case bhGameState::MENU:
					{
						EnterMode_Game();
						break;
					}
					default:
					{
						break;
					}
				}
				break;
			}
			case bhEvent::Type::EVT_QUIT:
			{
				state = bhGameState::QUIT;
				break;
			}
			case bhEvent::Type::EVT_TOGGLE_EDITOR:
			{
				if (state == bhGameState::EDITOR)
				{
					EnterMode_Game();
				}
				else
				{
					EnterMode_Editor();
				}
				break;
			}
			default:
			{
				world.HandleEvent(evt);
				break;
			}
		}
	}
}

void bhGame::MainLoop()
{
	//uint64_t lastTime = SDL_GetTicks64();
	uint64_t now = 0;
	static constexpr float TIME_MOD = 1.0f / 1000.0f;

	while (true)
	{
		switch (state)
		{
			case bhGameState::RUN:
			{
				bhInput::HandleSDLEvents();
				//now = SDL_GetTicks64();
				//bhTime_t tDiffSec = (now - lastTime) * TIME_MOD;
				world.Tick(0.0f);// tDiffSec);
				//lastTime = now;

				HandleBhEvents();

				bhVk::RenderDevice* rd = bhSystem::Get().RenderDevice();
				rd->BeginFrame();
				world.Render();
				rd->EndFrame();
				continue;
			}
			case bhGameState::MENU:
			{
				bhInput::HandleSDLEvents();
				HandleBhEvents();
				continue;
			}
			case bhGameState::EDITOR:
			{
				editor->HandleSDLEvents();

				bhVk::RenderDevice* rd = bhSystem::Get().RenderDevice();
				rd->BeginFrame();
				world.Render();
				editor->Render();
				rd->EndFrame();

				HandleBhEvents();
				continue;
			}
			case bhGameState::QUIT:
			{
				return;
			}
			case bhGameState::INIT:
			case bhGameState::READY:
			default:
			{
				continue;
			}
		}
	}
}

int bhGame::Run(char* argv[])
{
	bhGame game;
	if (game.Init(argv))
	{
		game.state = bhGame::bhGameState::RUN;
		bhInput::Enable();
		game.MainLoop();
		game.Destroy();
		return 0;
	}
	return -1;
}
