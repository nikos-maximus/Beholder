#ifndef BH_GAME_HPP
#define BH_GAME_HPP

#include <memory>
#include "Game/bhWorld.hpp"
#include "Editor/bhEditor.hpp"

class bhGame
{
public:
	static int Run(char* argv[]);

protected:
	enum class bhGameState
	{
		QUIT,
		INIT,
		READY,
		RUN,
		MENU,
		EDITOR
	};

	bhGameState state{ bhGameState::INIT };
	//bhGameState prevState{ bhGameState::INIT };

private:
	bool Init(char* argv[]);
	void Destroy();
	void MainLoop();
	void HandleBhEvents();

	void EnterMode_Menu();
	void EnterMode_Game();
	void EnterMode_Editor();

	bhWorld world;
	std::unique_ptr<bhEditor> editor;
};

#endif //BH_GAME_HPP
