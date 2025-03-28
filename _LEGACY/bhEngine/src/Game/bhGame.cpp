#include <string>
#include <SDL3/SDL_events.h>
#include <backends/imgui_impl_sdl3.h>
#include "Game/bhGame.hpp"
#include "bhInput.hpp"
#include "bhSystem.hpp"

////////////////////////////////////////////////////////////////////////////////
bool bhGame::Init(char* argv[])
{
  if (!bhSystem::Init(argv))
  {
    return false;
  }
  //bhAudio::Play();
  //bhInput::RegisterKeyBinding(SDL_SCANCODE_ESCAPE, bhInput::CMD_ESCAPE, bhEvent::Type::EVT_ESCAPE, bhInput::EC_PRESS);
  bhInput::RegisterKeyBinding(SDL_SCANCODE_DELETE, bhInput::CMD_QUIT, bhEvent::Type::EVT_QUIT, bhInput::EC_PRESS);
  bhInput::RegisterKeyBinding(SDL_SCANCODE_INSERT, bhInput::CMD_TOGGLE_UI_OVERLAY, bhEvent::Type::EVT_TOGGLE_UI_OVERLAY, bhInput::EC_PRESS);
  bhInput::RegisterDefaultKeyBindings();
  //bhInput::SetMouseLook(true);

  //world.Init();
  //world.Load_Resource("Map01.bhm");
  //bhShowWindow(bhSystem::MainWindow());

  //if (!bhSystem::InitImGui(bhSystem::MainWindow()))
  //{
  //  return false;
  //}

  world.reset(new bhWorld());
  if (!world->Init())
  {
    //return false;
  }

  bhSystem::SetMouseMode_Look();

  return true;
}

void bhGame::Destroy()
{
  world->Destroy();
  //bhSystem::DestroyImGui();

  //bhInput::Disable();
  //world.Destroy();

  bhSystem::Destroy();
}

void bhGame::EnterMode_Menu()
{
  state = bhGameState::MENU;
  bhSystem::SetMouseMode_Cursor();
}

void bhGame::EnterMode_Game()
{
  state = bhGameState::RUN;
  bhSystem::SetMouseMode_Look();
}

void bhGame::EnterMode_Editor()
{
  state = bhGameState::EDITOR;
  bhSystem::SetMouseMode_Cursor();
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
      case bhEvent::Type::EVT_TOGGLE_UI_OVERLAY:
      {
        showUIOverlay = !showUIOverlay;
        //if (state == bhGameState::EDITOR)
        //{
        //  EnterMode_Game();
        //}
        //else
        //{
        //  EnterMode_Editor();
        //}
        break;
      }
      default:
      {
        //world.HandleEvent(evt);
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

  //bhGPUContext* gpuC = bhSystem::GPUContext();
  while (true)
  {
    switch (state)
    {
      case bhGameState::RUN:
      {
        bhInput::HandleSDLEvents(ImGui_ImplSDL3_ProcessEvent);
        //now = SDL_GetTicks64();
        //bhTime_t tDiffSec = (now - lastTime) * TIME_MOD;
        //world.Tick(0.0f);// tDiffSec);
        //lastTime = now;

        HandleBhEvents();
        bhSystem::BeginFrame();
        world->Render();
        
        if (showUIOverlay)
        {
          bhSystem::SetMouseMode_Cursor();
          bhSystem::BeginImGuiFrame();
          ImGui::ShowDemoWindow();
          bhSystem::EndImGuiFrame();
        }
        else bhSystem::SetMouseMode_Look();
        
        bhSystem::EndFrame();
        continue;
      }
      case bhGameState::MENU:
      {
        bhInput::HandleSDLEvents(ImGui_ImplSDL3_ProcessEvent);
        HandleBhEvents();
        continue;
      }
      //case bhGameState::EDITOR:
      //{
      //	bhSystem::BeginFrame();
      //	world.Render();
      //	editor->Render();
      //	bhSystem::EndFrame();

      //	HandleBhEvents();
      //	continue;
      //}
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
