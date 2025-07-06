#pragma once
#include <SDL3/SDL.h>

#include <vector>

#include "main.hpp"

class Engine {
   public:
	/*
	 STARTUP: Compute FoV, immediately goes to IDLE
	 IDLE: Waiting for input for player, if got goto PLAYER_TURN
	 PLAYER_TURN: Input received! Player AI processes the input, if spent go to OTHER_ACTORS_TURN, else return to IDLE
	 OTHER_ACTORS_TURN: Turn for actors other than the player, return to IDLE afterwards
	 MENU: Waiting for input for menu, goto MENU_UPDATE
	 MENU_UPDATE: Some menu processes it and go to OTHER_ACTORS_TURN (if count as an action for player), else return to
	 IDLE VICTORY / DEFEAT: End states
	*/
	enum GameStatus { STARTUP, IDLE, PLAYER_TURN, OTHER_ACTORS_TURN, MENU, MENU_UPDATE, VICTORY, DEFEAT } gameStatus;

	Engine() = default;

	static constexpr int MAP_WIDTH = 72;
	static constexpr int MAP_HEIGHT = 32;
	static constexpr int GUI_PANEL_HEIGHT = 8;
	static constexpr int CONSOLE_WIDTH = MAP_WIDTH;
	static constexpr int CONSOLE_HEIGHT = MAP_HEIGHT + GUI_PANEL_HEIGHT;

	Uint32 lastEventType;
	SDL_KeyboardEvent lastKeyboardEvent;
	Uint8 lastMouseButton;
	int lastMouseTileX, lastMouseTileY;

	SDL_AppResult init(int argc, char** argv);
	SDL_AppResult iterate();
	SDL_AppResult handleEvent(const SDL_Event& event);
	void shutdown();

	void removeActor(Actor* actor);
	void sendToBack(Actor* actor);
	Actor* getActor(int x, int y) const;
	Actor* getClosestMonster(int x, int y, float range) const;

	// List of actors that will be rendered and updated each frame or turn, including player, item on ground, etc.
	// Memories of these will be released on destructing the engine class
	std::vector<Actor*> actors;
	Actor* player;

	Map* map;
	Gui* gui;

	int fovRadius;

	~Engine();

   private:
	std::filesystem::path getDataDir();
	void render(tcod::Console& console);

	tcod::Console console;
	tcod::Context context;

	bool computeFov;
};

extern Engine engine;
