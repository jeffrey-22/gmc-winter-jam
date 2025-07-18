#include <cassert>
#include <filesystem>
#include <string>

#include "main.hpp"

static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};
static constexpr auto RED = tcod::ColorRGB{255, 0, 0};
static constexpr auto LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};

std::filesystem::path Engine::getDataDir() {
	auto current = std::filesystem::current_path();
	while (!std::filesystem::exists(current / "data")) {
		auto parent = current.parent_path();
		if (parent == current) {
			throw std::runtime_error("Could not find the data directory.");
		}
		current = parent;
	}
	return current / "data";
}

// Configure param settings and initialize members
SDL_AppResult Engine::init(int argc, char** argv) {
	auto params = TCOD_ContextParams{};
	params.argc = argc;
	params.argv = argv;
	params.renderer_type = TCOD_RENDERER_SDL2;
	params.vsync = 1;
	params.sdl_window_flags = SDL_WINDOW_RESIZABLE;
	params.window_title = "The Underworlder";

	// Load tileset
	auto tileset = tcod::load_tilesheet(getDataDir() / "Zesty_curses_24x24.png", {16, 16}, tcod::CHARMAP_CP437);
	params.tileset = tileset.get();

	// Load console
	console = tcod::Console{CONSOLE_WIDTH, CONSOLE_HEIGHT};
	params.console = console.get();

	// Load context
	context = tcod::Context(params);

	// Create actors
	player = new Actor(console.get_width() / 2, console.get_height() / 2, '@', "player", {200, 210, 220});
	player->destructible = new PlayerDestructible(50, 2, "your cadaver");
	player->attacker = new Attacker(35);
	player->ai = new PlayerAi();
	player->container = new Container(36);
	actors.push_back(player);

	createNatureActor();

	level = 1;
	stairs = new Actor(0, 0, '>', "stairs", WHITE);
	stairs->blocks = false;
	stairs->fovOnly = false;
	actors.push_back(stairs);

	// Create map (after actors)
	map = new Map(MAP_WIDTH, MAP_HEIGHT);

	// Make stairs the last actor
	sendToBack(stairs);

	// Create Gui
	gui = new Gui();

	nameTracker = new NameTracker(new Random());

	// Initialize other variables
	fovRadius = 10;
	monsterSpawnRate = 50;
	gameStatus = STARTUP;
	lastMouseTileX = lastMouseTileY = 0;

	return SDL_APP_CONTINUE;
}

// Render console graphics, including map, actors and gui
void Engine::render(tcod::Console& console) {
	// Render map tiles
	map->render(console);

	// Render actors if in FoV
	for (auto actor : actors)
		if ((!actor->fovOnly && map->isExplored(actor->x, actor->y)) || map->isInFov(actor->x, actor->y) ||
			map->isMapRevealed)
			actor->render(console);

	// Render Gui elements (on top, or modify the base console colors)
	gui->render(console);

	if (gameStatus == VICTORY) {
		for (int x = 0; x < CONSOLE_WIDTH; x++)
			for (int y = 0; y < CONSOLE_HEIGHT; y++) {
				console.at({x, y}).bg.r = (uint8_t)std::min(winEffect + console.at({x, y}).bg.r, 255.0F);
				console.at({x, y}).bg.g = (uint8_t)std::min(winEffect + console.at({x, y}).bg.g, 255.0F);
				console.at({x, y}).bg.b = (uint8_t)std::min(winEffect + console.at({x, y}).bg.b, 255.0F);
			}
	}
}

// Called every frame, update actor turns if new turn, then render console graphics
SDL_AppResult Engine::iterate() {
	console.clear();

	// Update FoV if needed, currently only on first frame and on player movement
	if (gameStatus == STARTUP) {
		map->computeFov();
		gui->message("Welcome stranger!\nPress '?' to view controls.", RED);
		new IntroductionMenu();
		gameStatus = MENU;
	} else if (gameStatus == PLAYER_TURN) {
		// If turn is spent go to OTHER_ACTORS_TURN, else return to idle
		player->update();  // status updated inside playerAi
	} else if (gameStatus == OTHER_ACTORS_TURN) {
		for (auto actor : actors)
			if (actor != player) actor->update();
		gameStatus = IDLE;
	} else if (gameStatus == MENU_UPDATE) {
		// Status updated inside
		gui->update();
	} else if (gameStatus == VICTORY) {
		winEffect += 0.2F;
	}

	// Render graphics for this frame
	render(console);

	// Update context with console
	context.present(console);

	return SDL_APP_CONTINUE;
}

// Handle events like inputs
SDL_AppResult Engine::handleEvent(const SDL_Event& event) {
	lastEventType = event.type;

	if (event.type == SDL_EVENT_KEY_DOWN) {
		lastKeyboardEvent = event.key;
	} else if (event.type == SDL_EVENT_MOUSE_MOTION) {
		auto mouseEvent = event;
		context.convert_event_coordinates(mouseEvent);
		auto mouseTile = mouseEvent.motion;
		lastMouseTileX = (int)mouseTile.x;
		lastMouseTileY = (int)mouseTile.y;
	} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		lastMouseButton = event.button.button;
	} else if (event.type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}

	if (event.type == SDL_EVENT_KEY_DOWN || SDL_EVENT_MOUSE_MOTION || SDL_EVENT_MOUSE_BUTTON_DOWN) {
		// If game state is waiting for input, set state to wake them up to process the input
		if (gameStatus == IDLE) {
			gameStatus = PLAYER_TURN;
		} else if (gameStatus == MENU) {
			gameStatus = MENU_UPDATE;
		}
	}
	return SDL_APP_CONTINUE;
}

// Remove actor from the main actor list, note it's not deleted here
void Engine::removeActor(Actor* actor) {
	auto it = std::find(actors.begin(), actors.end(), actor);
	assert(it != actors.end());
	actors.erase(it);
}

// Reorder actor to the beginning of the list, i.e. rendered at the back
void Engine::sendToBack(Actor* actor) {
	auto it = std::find(actors.begin(), actors.end(), actor);
	assert(it != actors.end());
	actors.erase(it);
	actors.insert(actors.begin(), actor);
}

// Return an alive actor, including the player, at (x, y). Returns NULL if not found.
Actor* Engine::getActor(int x, int y) const {
	for (auto actor : actors) {
		if (actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead()) {
			return actor;
		}
	}
	return NULL;
}

// Returns the closest alive monster from position x,y within range. If range is 0, it's considered infinite. If
// no monster is found within range, it returns NULL
Actor* Engine::getClosestMonster(int x, int y, float range) const {
	Actor* closest = NULL;
	float bestDistance = 1E6f;
	for (auto actor : actors) {
		if (actor != player && actor->destructible && !actor->destructible->isDead()) {
			float distance = actor->getDistance(x, y);
			if (distance < bestDistance && (distance <= range || range == 0.0f)) {
				bestDistance = distance;
				closest = actor;
			}
		}
	}
	return closest;
}

void Engine::createNatureActor() {
	nature = new Actor(-1, -1, ' ', "Nature", RED);
	nature->ai = new NatureAi(level);
	actors.push_back(nature);
}

void Engine::nextLevel() {
	if (level == 20) {
		gui->message("Congratulations!\nYou found the exit and escaped!", LIGHT_BLUE);
		gameStatus = VICTORY;
		winEffect = 0.0;
		return;
	}
	level++;
	if (level == 11)
		monsterSpawnRate = 40;
	else if (level >= 19)
		monsterSpawnRate = 30;
	if (level == 11)
		fovRadius = 9;
	else if (level == 12)
		fovRadius = 8;
	else if (level == 14)
		fovRadius = 7;
	else if (level == 17)
		fovRadius = 6;
	else if (level == 19)
		fovRadius = 5;
	gui->message("You descended deeper...", LIGHT_BLUE);
	// Regenerate map
	delete map;
	// Delete all actors but player and stairs
	std::vector<Actor*> actorsToBeDeleted = {};
	for (auto actor : actors)
		if (actor != player && actor != stairs) actorsToBeDeleted.push_back(actor);
	for (auto actor : actorsToBeDeleted) removeActor(actor);
	// Create a new map
	map = new Map(MAP_WIDTH, MAP_HEIGHT);
	sendToBack(stairs);
	createNatureActor();
	map->computeFov();
	gameStatus = IDLE;
}

// Called on windows exit
void Engine::shutdown() {}

// Destructor
Engine::~Engine() {
	// Free actor memories
	for (auto actor : actors) {
		delete actor;
	}

	// Free map memories
	delete map;

	// Free Gui
	delete gui;
}
