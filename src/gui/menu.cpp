#include <cassert>
#include <iostream>

#include "main.hpp"

static const int INVENTORY_WIDTH = 50;
static const int INVENTORY_HEIGHT = 28;

static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

void Menu::closeWithState(Engine::GameStatus newStatus) {
	engine.gui->isMenuOpen = false;
	engine.gui->menu = NULL;
	engine.gameStatus = newStatus;
	delete this;
}

void Menu::closeWithMenuSwitch(Menu* newMenu) {
	engine.gui->menu = newMenu;
	engine.gui->isMenuOpen = true;
	engine.gameStatus = Engine::MENU;
	delete this;
}

InventoryMenu::InventoryMenu(Actor* inventoryOwner)
	: Menu(), inventoryConsole(tcod::Console{INVENTORY_WIDTH, INVENTORY_HEIGHT}), inventoryOwner(inventoryOwner) {
	// Prepare inventory console
	inventoryConsole.clear();
	tcod::print_rect(
		inventoryConsole, {0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT}, "Inventory", LIGHT_GREY, std::nullopt);
	assert(inventoryOwner->container);

	int shortcut = 'a';
	int y = 1;
	for (auto actor : inventoryOwner->container->inventory) {
		tcod::print(inventoryConsole, {2, y}, tcod::stringf("(%c) %s", shortcut, actor->name), WHITE, std::nullopt);
		y++;
		shortcut++;
	}
}

void InventoryMenu::update() {
	bool gotAtoZ = false;
	bool gotEsc = false;
	char gotAscii;

	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN && engine.lastMouseButton == SDL_BUTTON_RIGHT) {
		// Right click closes the inventory
		closeWithState(Engine::IDLE);
		return;
	}

	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Not effective input, keep menu state idle.
		engine.gameStatus = Engine::MENU;
		return;
	}

	if (engine.lastKeyboardEvent.key >= SDLK_A && engine.lastKeyboardEvent.key <= SDLK_Z) {
		gotAtoZ = true;
		gotAscii = engine.lastKeyboardEvent.key - SDLK_A + 'a';
	} else {
		switch (engine.lastKeyboardEvent.key) {
			case SDLK_ESCAPE:
				gotEsc = true;
				break;
			default:
				break;
		}
	}

	if (gotEsc) {
		closeWithState(Engine::IDLE);
		return;
	}
	if (gotAtoZ) {
		// Got A-Z, check if its a usable item, if so use it and continue to OTHER_ACTORS_TURN
		int itemIndex = gotAscii - 'a';
		if (inventoryOwner->container->isIndexValid(itemIndex)) {
			auto itemActor = inventoryOwner->container->inventory[itemIndex];
			// Try to use it (and spend the turn) regardless of its outcome
			Menu* nextMenu = itemActor->pickable->use(itemActor, inventoryOwner, this);
			if (nextMenu == NULL) {
				// Item used and turn spent successfully
				closeWithState(Engine::OTHER_ACTORS_TURN);
				return;
			} else if (nextMenu == this) {
				// Usage failed, back to inventory screen
				engine.gameStatus = Engine::MENU;
				return;
			} else {
				// Switch to new menu
				closeWithMenuSwitch(nextMenu);
				return;
			}
		}
	}
	// Otherwise, not sure what to do, but just keep it open and ask user to give a new input
	engine.gameStatus = Engine::MENU;
}

void InventoryMenu::render(tcod::Console& mainConsole) {
	tcod::blit(
		mainConsole,
		inventoryConsole,
		{80 / 2 - INVENTORY_WIDTH / 2, 40 / 2 - INVENTORY_HEIGHT / 2},
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT});
}

TilePickMenu::TilePickMenu(Pickable* invoker, Actor* owner, Actor* wearer, bool allowCancel, TilePickRange pickRange)
	: invoker(invoker), owner(owner), wearer(wearer), allowCancel(allowCancel), pickRange(pickRange) {}

void TilePickMenu::handleCancel() {
	if (allowCancel) {
		Menu* nextMenu = invoker->tilePickCallback(owner, wearer, true, 0, 0, this);
		if (nextMenu == NULL) {
			// Turn spent successfully
			closeWithState(Engine::OTHER_ACTORS_TURN);
			return;
		} else if (nextMenu == this) {
			// Usage failed, back to menu waiting for a new input
			engine.gameStatus = Engine::MENU;
			return;
		} else {
			// Switch to new menu
			closeWithMenuSwitch(nextMenu);
			return;
		}
	} else {
		engine.gameStatus = Engine::MENU;
		return;
	}
}

void TilePickMenu::update() {
	if (engine.lastEventType == SDL_EVENT_KEY_DOWN) {
		// Regarded as cancellation attempts if got keyboard inputs
		handleCancel();
		return;
	}
	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		if (engine.lastMouseButton == SDL_BUTTON_LEFT) {
			int x = engine.lastMouseTileX, y = engine.lastMouseTileY;
			if (engine.map->isInFov(x, y) && (maxRange == 0 || engine.player->getDistance(x, y) <= maxRange)) {
				Menu* nextMenu = invoker->tilePickCallback(owner, wearer, false, x, y, this);
				if (nextMenu == NULL) {
					// Turn spent successfully
					closeWithState(Engine::OTHER_ACTORS_TURN);
					return;
				} else if (nextMenu == this) {
					// Usage failed, back to menu waiting for a new input
					engine.gameStatus = Engine::MENU;
					return;
				} else {
					// Switch to new menu
					closeWithMenuSwitch(nextMenu);
					return;
				}
			}
		} else {
			// Not left click
			handleCancel();
			return;
		}
	}
	engine.gameStatus = Engine::MENU;
}

void TilePickMenu::render(tcod::Console& mainConsole) {
	Map* map = engine.map;
	for (int cx = 0; cx < map->width; cx++) {
		for (int cy = 0; cy < map->height; cy++) {
			if (engine.map->isInFov(cx, cy) && (maxRange == 0 || owner->getDistance(cx, cy) <= maxRange)) {
				TCOD_ColorRGBA col = mainConsole.at({cx, cy}).bg;

				// Highlight selectable tiles by multiplying 1.2
				float scalar = 1.2f;
				auto scale = [&](uint8_t component) -> uint8_t {
					return static_cast<uint8_t>(std::clamp(static_cast<int>(component * scalar), 0, 255));
				};
				col = {scale(col.r), scale(col.g), scale(col.b), scale(col.a)};

				mainConsole.at({cx, cy}).bg = col;
			}
		}
	}
}

ItemPickMenu::ItemPickMenu(Pickable* invoker, Actor* owner, Actor* wearer, bool allowCancel)
	: invoker(invoker),
	  owner(owner),
	  wearer(wearer),
	  allowCancel(allowCancel),
	  itemPickConsole(tcod::Console{INVENTORY_WIDTH, INVENTORY_HEIGHT}) {
	// Prepare item picking console
	itemPickConsole.clear();
	tcod::print_rect(
		itemPickConsole,
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT},
		"Pick an item, Right click to cancel:",
		LIGHT_GREY,
		std::nullopt);

	invokerShortcutChar = -1;
	int shortcut = 'a';
	int y = 1;
	for (auto actor : wearer->container->inventory) {
		auto color = WHITE;
		if (actor == owner) {
			invokerShortcutChar = shortcut;
			color = LIGHT_GREY;
		}
		tcod::print(itemPickConsole, {2, y}, tcod::stringf("(%c) %s", shortcut, actor->name), color, std::nullopt);
		y++;
		shortcut++;
	}
}

void ItemPickMenu::handleCancel() {
	if (allowCancel) {
		Menu* nextMenu = invoker->actorPickCallback(owner, wearer, true, NULL, this);
		if (nextMenu == NULL) {
			// Turn spent successfully
			closeWithState(Engine::OTHER_ACTORS_TURN);
			return;
		} else if (nextMenu == this) {
			// Usage failed, back to menu waiting for a new input
			engine.gameStatus = Engine::MENU;
			return;
		} else {
			// Switch to new menu
			closeWithMenuSwitch(nextMenu);
			return;
		}
	} else {
		engine.gameStatus = Engine::MENU;
		return;
	}
}

void ItemPickMenu::update() {
	bool gotAtoZ = false;
	bool gotEsc = false;
	char gotAscii;

	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN && engine.lastMouseButton == SDL_BUTTON_RIGHT) {
		handleCancel();
		return;
	}

	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Not effective input, keep menu state idle.
		engine.gameStatus = Engine::MENU;
		return;
	}

	if (engine.lastKeyboardEvent.key >= SDLK_A && engine.lastKeyboardEvent.key <= SDLK_Z) {
		gotAtoZ = true;
		gotAscii = engine.lastKeyboardEvent.key - SDLK_A + 'a';
	} else {
		switch (engine.lastKeyboardEvent.key) {
			case SDLK_ESCAPE:
				gotEsc = true;
				break;
			default:
				break;
		}
	}

	if (gotEsc) {
		handleCancel();
		return;
	}
	if (gotAtoZ) {
		// Got A-Z, check if its a usable item, if so use it and continue to OTHER_ACTORS_TURN
		int itemIndex = gotAscii - 'a';
		if (wearer->container->isIndexValid(itemIndex) && itemIndex + 'a' != invokerShortcutChar) {
			auto itemActor = wearer->container->inventory[itemIndex];
			Menu* nextMenu = invoker->actorPickCallback(owner, wearer, false, itemActor, this);
			if (nextMenu == NULL) {
				// Item used and turn spent successfully
				closeWithState(Engine::OTHER_ACTORS_TURN);
				return;
			} else if (nextMenu == this) {
				// Usage failed, wait for new effective input
				engine.gameStatus = Engine::MENU;
				return;
			} else {
				// Switch to new menu
				closeWithMenuSwitch(nextMenu);
				return;
			}
		}
	}
	engine.gameStatus = Engine::MENU;
}

void ItemPickMenu::render(tcod::Console& mainConsole) {
	tcod::blit(
		mainConsole,
		itemPickConsole,
		{80 / 2 - INVENTORY_WIDTH / 2, 40 / 2 - INVENTORY_HEIGHT / 2},
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT});
}

ItemDropMenu::ItemDropMenu(Actor* inventoryOwner)
	: inventoryOwner(inventoryOwner), itemDropConsole(tcod::Console{INVENTORY_WIDTH, INVENTORY_HEIGHT}) {
	itemDropConsole.clear();
	tcod::print_rect(
		itemDropConsole,
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT},
		"Drop which? Right click to cancel",
		LIGHT_GREY,
		std::nullopt);
	assert(inventoryOwner->container);

	int shortcut = 'a';
	int y = 1;
	for (auto actor : inventoryOwner->container->inventory) {
		tcod::print(itemDropConsole, {2, y}, tcod::stringf("(%c) %s", shortcut, actor->name), WHITE, std::nullopt);
		y++;
		shortcut++;
	}
}

void ItemDropMenu::update() {
	bool gotAtoZ = false;
	bool gotEsc = false;
	char gotAscii;

	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN && engine.lastMouseButton == SDL_BUTTON_RIGHT) {
		// Right click closes the drop menu
		closeWithState(Engine::IDLE);
		return;
	}

	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Not effective input, keep menu state idle.
		engine.gameStatus = Engine::MENU;
		return;
	}

	if (engine.lastKeyboardEvent.key >= SDLK_A && engine.lastKeyboardEvent.key <= SDLK_Z) {
		gotAtoZ = true;
		gotAscii = engine.lastKeyboardEvent.key - SDLK_A + 'a';
	} else {
		switch (engine.lastKeyboardEvent.key) {
			case SDLK_ESCAPE:
				gotEsc = true;
				break;
			default:
				break;
		}
	}

	if (gotEsc) {
		closeWithState(Engine::IDLE);
		return;
	}
	if (gotAtoZ) {
		// Got A-Z, check if its a droppable item, if so drop it and continue to OTHER_ACTORS_TURN
		int itemIndex = gotAscii - 'a';
		if (inventoryOwner->container->isIndexValid(itemIndex)) {
			auto itemActor = inventoryOwner->container->inventory[itemIndex];
			// Drop it (and spend the turn)
			itemActor->pickable->drop(itemActor, inventoryOwner);
			engine.gameStatus = Engine::OTHER_ACTORS_TURN;
			closeWithState(Engine::OTHER_ACTORS_TURN);
			return;
		}
	}
	// Otherwise, not sure what to do, but just keep it open and ask user to give a new input
	engine.gameStatus = Engine::MENU;
}

void ItemDropMenu::render(tcod::Console& mainConsole) {
	tcod::blit(
		mainConsole,
		itemDropConsole,
		{80 / 2 - INVENTORY_WIDTH / 2, 40 / 2 - INVENTORY_HEIGHT / 2},
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT});
}
