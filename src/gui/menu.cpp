#include <cassert>

#include "main.hpp"

static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};
static constexpr auto DARK_RED = tcod::ColorRGB{45, 23, 23};
static constexpr auto BLACK = tcod::ColorRGB{0, 0, 0};
static constexpr auto ORANGE = tcod::ColorRGB{255, 159, 63};
static constexpr auto DARKER_BLUE = tcod::ColorRGB{0, 0, 127};
static constexpr auto DARKER_GREEN = tcod::ColorRGB{0, 127, 0};

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
	: Menu(),
	  inventoryConsole(tcod::Console{INVENTORY_WIDTH, INVENTORY_HEIGHT}),
	  inventoryOwner(inventoryOwner),
	  selectedIndex(-1),
	  onCallMenu(false),
	  callString("") {}

void InventoryMenu::update() {
	// handle if in the middle of calling an item
	if (onCallMenu) {
		if (engine.lastKeyboardEvent.key == SDLK_ESCAPE ||
			engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN && engine.lastMouseButton == SDL_BUTTON_RIGHT) {
			onCallMenu = false;
			callString = "";
			engine.gameStatus = Engine::MENU;
			return;
		}
		if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
			// Not effective input, keep menu state idle.
			engine.gameStatus = Engine::MENU;
			return;
		}
		if (engine.lastKeyboardEvent.key == SDLK_KP_ENTER || engine.lastKeyboardEvent.key == SDLK_RETURN) {
			// Confirm change
			if (inventoryOwner->container->isIndexValid(selectedIndex))
				engine.nameTracker->callItem(inventoryOwner->container->inventory[selectedIndex], callString);
			onCallMenu = false;
			callString = "";
			engine.gameStatus = Engine::MENU;
			return;
		}
		bool gotValid = false;
		char gotAscii;
		if (engine.lastKeyboardEvent.key >= SDLK_A && engine.lastKeyboardEvent.key <= SDLK_Z) {
			gotValid = true;
			gotAscii = engine.lastKeyboardEvent.key - SDLK_A + 'a';
		} else if (engine.lastKeyboardEvent.key >= SDLK_0 && engine.lastKeyboardEvent.key <= SDLK_9) {
			gotValid = true;
			gotAscii = engine.lastKeyboardEvent.key - SDLK_0 + '0';
		} else if (engine.lastKeyboardEvent.key == SDLK_SPACE) {
			gotValid = true;
			gotAscii = ' ';
		}
		if (gotValid && callString.length() < engine.nameTracker->MAX_CALL_STRING_LENGTH) {
			callString.push_back(gotAscii);
		} else if (engine.lastKeyboardEvent.key == SDLK_BACKSPACE && !callString.empty()) {
			callString.pop_back();
		}
		engine.gameStatus = Engine::MENU;
		return;
	}
	// Handle if theres already an item selected (index not -1)
	if (selectedIndex != -1) {
		if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN && engine.lastMouseButton == SDL_BUTTON_RIGHT) {
			// Mouse clicks, cancel selection.
			selectedIndex = -1;
			engine.gameStatus = Engine::MENU;
			return;
		}
		if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
			// Not effective input, keep menu state idle.
			engine.gameStatus = Engine::MENU;
			return;
		}
		bool gotDrop = false;
		bool gotApply = false;
		bool gotCall = false;
		bool gotEsc = false;
		switch (engine.lastKeyboardEvent.key) {
			case SDLK_A:
				gotApply = true;
				break;
			case SDLK_C:
				gotCall = true;
				break;
			case SDLK_D:
				gotDrop = true;
				break;
			case SDLK_I:
			case SDLK_ESCAPE:
				gotEsc = true;
				break;
			default:
				break;
		}
		if (gotApply) {
			useItemAtIndex(selectedIndex);
			return;
		} else if (gotDrop) {
			if (inventoryOwner->container->isIndexValid(selectedIndex)) {
				auto itemActor = inventoryOwner->container->inventory[selectedIndex];
				Actor* groundItemActor = NULL;
				for (auto actor : engine.actors)
					if (actor->pickable && actor->x == inventoryOwner->x && actor->y == inventoryOwner->y) {
						groundItemActor = actor;
						break;
					}
				if (groundItemActor == NULL) {
					// Drop it (and spend the turn)
					itemActor->pickable->drop(itemActor, inventoryOwner);
				} else {
					// Swap with ground item
					itemActor->pickable->swap(itemActor, groundItemActor, inventoryOwner);
				}
				engine.gameStatus = Engine::OTHER_ACTORS_TURN;
				closeWithState(Engine::OTHER_ACTORS_TURN);
				return;
			}
		} else if (gotCall) {
			callString = "";
			onCallMenu = true;
			engine.gameStatus = Engine::MENU;
			return;
		} else if (gotEsc) {
			selectedIndex = -1;
			engine.gameStatus = Engine::MENU;
			return;
		}
		engine.gameStatus = Engine::MENU;
		return;
	}

	bool gotAtoZ = false;
	bool got0to9 = false;
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
	} else if (engine.lastKeyboardEvent.key >= SDLK_0 && engine.lastKeyboardEvent.key <= SDLK_9) {
		got0to9 = true;
		gotAscii = engine.lastKeyboardEvent.key - SDLK_0 + '0';
	} else {
		switch (engine.lastKeyboardEvent.key) {
			case SDLK_KP_0:
				got0to9 = true;
				gotAscii = '0';
				break;
			case SDLK_KP_1:
				got0to9 = true;
				gotAscii = '1';
				break;
			case SDLK_KP_2:
				got0to9 = true;
				gotAscii = '2';
				break;
			case SDLK_KP_3:
				got0to9 = true;
				gotAscii = '3';
				break;
			case SDLK_KP_4:
				got0to9 = true;
				gotAscii = '4';
				break;
			case SDLK_KP_5:
				got0to9 = true;
				gotAscii = '5';
				break;
			case SDLK_KP_6:
				got0to9 = true;
				gotAscii = '6';
				break;
			case SDLK_KP_7:
				got0to9 = true;
				gotAscii = '7';
				break;
			case SDLK_KP_8:
				got0to9 = true;
				gotAscii = '8';
				break;
			case SDLK_KP_9:
				got0to9 = true;
				gotAscii = '9';
				break;
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
	if (gotAtoZ || got0to9) {
		// Got A-Z 0-9, check if its a usable item, if so select it
		int itemIndex;
		if (gotAtoZ)
			itemIndex = gotAscii - 'a';
		else {
			if (gotAscii == '0')
				itemIndex = 35;
			else
				itemIndex = gotAscii - '1' + 26;
		}
		if (inventoryOwner->container->isIndexValid(itemIndex)) {
			selectedIndex = itemIndex;
			engine.gameStatus = Engine::MENU;
			return;
		} else {
			engine.gameStatus = Engine::MENU;
			return;
		}
	}
	// Otherwise, not sure what to do, but just keep it open and ask user to give a new input
	engine.gameStatus = Engine::MENU;
}

// Use item, changing gameStatus. Must immediately return after calling this.
void InventoryMenu::useItemAtIndex(int itemIndex) {
	auto itemActor = inventoryOwner->container->inventory[itemIndex];
	// Identify it
	engine.nameTracker->identifyItem(itemActor);
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
	engine.gameStatus = Engine::MENU;
}

void InventoryMenu::renderInventory() {
	// Prepare inventory console
	inventoryConsole.clear();
	tcod::print_rect(
		inventoryConsole, {0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT}, "Inventory", LIGHT_GREY, std::nullopt);
	assert(inventoryOwner->container);

	int shortcut = 'a';
	int y = 1;
	for (auto actor : inventoryOwner->container->inventory) {
		std::optional<TCOD_ColorRGB> backgroundColor = std::nullopt;
		if (selectedIndex == y - 1) backgroundColor = ORANGE;
		tcod::print(
			inventoryConsole,
			{2, y},
			tcod::stringf("(%c) %c %s", shortcut, actor->ch, engine.nameTracker->getDisplayName(actor)),
			WHITE,
			backgroundColor);
		y++;
		shortcut++;
		if (y == 27)
			shortcut = '1';
		else if (y == 36)
			shortcut = '0';
	}
}

void InventoryMenu::render(tcod::Console& mainConsole) {
	renderInventory();

	tcod::blit(
		mainConsole,
		inventoryConsole,
		{engine.CONSOLE_WIDTH / 2 - INVENTORY_WIDTH / 2, engine.CONSOLE_HEIGHT / 2 - INVENTORY_HEIGHT / 2},
		{0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT},
		0.95F,
		0.6F);

	if (selectedIndex != -1) {
		int maxLength = 0;
		Actor* itemActor = inventoryOwner->container->inventory[selectedIndex];
		std::string displayName = std::string(engine.nameTracker->getDisplayName(itemActor));
		std::string description = std::string(engine.nameTracker->getDescription(itemActor));
		std::string controlsText = "[A] Apply  [D] Drop  [C] Call";
		maxLength = std::max(maxLength, (int)displayName.size());
		maxLength = std::max(maxLength, (int)controlsText.size());
		int currentLength = 0;
		for (char ch : description) {
			if (ch == '\n') {
				maxLength = std::max(maxLength, currentLength);
				currentLength = 0;
			} else
				++currentLength;
		}
		maxLength = std::max(maxLength, currentLength);
		int heightNeeded = 3;
		for (char ch : description)
			if (ch == '\n') heightNeeded++;
		int ySubMenu = engine.CONSOLE_HEIGHT / 2 - INVENTORY_HEIGHT / 2 + selectedIndex + 2;
		if (ySubMenu + heightNeeded - 1 >= INVENTORY_HEIGHT) {
			ySubMenu -= heightNeeded + 1;
			ySubMenu = std::max(ySubMenu, 0);
		}
		int xSubMenu = 6;
		tcod::draw_rect(mainConsole, {xSubMenu, ySubMenu, maxLength, heightNeeded}, 0, DARKER_BLUE, DARKER_BLUE);
		tcod::print(mainConsole, {xSubMenu, ySubMenu}, displayName, WHITE, DARKER_BLUE);
		tcod::print(mainConsole, {xSubMenu, ySubMenu + 1}, description, WHITE, DARKER_BLUE);
		tcod::print(mainConsole, {xSubMenu, ySubMenu + heightNeeded - 1}, controlsText, WHITE, DARKER_BLUE);
	}

	if (onCallMenu) {
		int maxLength = 0;
		Actor* itemActor = inventoryOwner->container->inventory[selectedIndex];
		std::string hintString = tcod::stringf("Enter new name for %s:", engine.nameTracker->getDisplayName(itemActor));
		std::string controlsText = "[Enter] Confirm  [Esc] Cancel";
		maxLength = std::max(maxLength, (int)hintString.length());
		maxLength = std::max(maxLength, (int)callString.length());
		maxLength = std::max(maxLength, (int)controlsText.length());
		int xSubMenu = engine.CONSOLE_WIDTH / 2 - maxLength / 2;
		int ySubMenu = engine.CONSOLE_HEIGHT / 2 - 1;
		tcod::draw_rect(mainConsole, {xSubMenu, ySubMenu, maxLength, 3}, 0, DARKER_GREEN, DARKER_GREEN);
		tcod::print(mainConsole, {xSubMenu, ySubMenu}, hintString, WHITE, DARKER_GREEN);
		tcod::print(mainConsole, {xSubMenu, ySubMenu + 1}, callString, WHITE, DARKER_GREEN);
		tcod::print(mainConsole, {xSubMenu, ySubMenu + 2}, controlsText, WHITE, DARKER_GREEN);
	}
}

TilePickMenu::TilePickMenu(
	TargetSelector* invoker, Actor* owner, Actor* wearer, bool allowCancel, TilePickRange pickRange)
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
			if (engine.map->isInFov(x, y) && (maxRange == 0 || wearer->getDistance(x, y) <= maxRange) &&
				engine.map->isWalkable(x, y)) {
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
			if (engine.map->isInFov(cx, cy) && (maxRange == 0 || wearer->getDistance(cx, cy) <= maxRange) &&
				engine.map->isWalkable(cx, cy)) {
				TCOD_ColorRGBA col = mainConsole.at({cx, cy}).bg;

				// Highlight selectable tiles by greenifying
				float p = 0.5f;
				TCOD_ColorRGBA hiCol;
				hiCol.r = 120;
				hiCol.g = 255;
				hiCol.b = 120;
				hiCol.a = 255;
				if (engine.lastMouseTileX == cx && engine.lastMouseTileY == cy) {
					hiCol.r = 120;
					hiCol.g = 120;
					hiCol.b = 255;
					hiCol.a = 255;
				}
				col = {
					static_cast<uint8_t>(col.r * p + hiCol.r * (1 - p)),
					static_cast<uint8_t>(col.g * p + hiCol.g * (1 - p)),
					static_cast<uint8_t>(col.b * p + hiCol.b * (1 - p)),
					col.a};
				mainConsole.at({cx, cy}).bg = col;
			}
		}
	}
}

ItemPickMenu::ItemPickMenu(TargetSelector* invoker, Actor* owner, Actor* wearer, bool allowCancel)
	: invoker(invoker),
	  owner(owner),
	  wearer(wearer),
	  allowCancel(allowCancel),
	  itemPickConsole(tcod::Console{InventoryMenu::INVENTORY_WIDTH, InventoryMenu::INVENTORY_HEIGHT}) {
	// Prepare item picking console
	itemPickConsole.clear();
	tcod::print_rect(
		itemPickConsole,
		{0, 0, InventoryMenu::INVENTORY_WIDTH, InventoryMenu::INVENTORY_HEIGHT},
		"Pick an item, Right click to cancel:",
		LIGHT_GREY,
		std::nullopt);

	invokerShortcutIndex = -1;
	int shortcut = 'a';
	int y = 1;
	for (auto actor : wearer->container->inventory) {
		auto color = WHITE;
		if (actor == owner) {
			invokerShortcutIndex = y - 1;
			color = LIGHT_GREY;
		}
		tcod::print(
			itemPickConsole,
			{2, y},
			tcod::stringf("(%c) %c %s", shortcut, actor->ch, engine.nameTracker->getDisplayName(actor)),
			color,
			std::nullopt);
		y++;
		shortcut++;
		if (y == 27)
			shortcut = '1';
		else if (y == 36)
			shortcut = '0';
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
	bool got0to9 = false;
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
	} else if (engine.lastKeyboardEvent.key >= SDLK_0 && engine.lastKeyboardEvent.key <= SDLK_9) {
		got0to9 = true;
		gotAscii = engine.lastKeyboardEvent.key - SDLK_0 + '0';
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
		// Got A-Z 0-9, check if its a usable item, if so use it and continue to OTHER_ACTORS_TURN
		int itemIndex;
		if (gotAtoZ)
			itemIndex = gotAscii - 'a';
		else {
			if (gotAscii == '0')
				itemIndex = 35;
			else
				itemIndex = gotAscii - '1' + 26;
		}
		if (wearer->container->isIndexValid(itemIndex) && itemIndex != invokerShortcutIndex) {
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
		{engine.CONSOLE_WIDTH / 2 - InventoryMenu::INVENTORY_WIDTH / 2,
		 engine.CONSOLE_HEIGHT / 2 - InventoryMenu::INVENTORY_HEIGHT / 2},
		{0, 0, InventoryMenu::INVENTORY_WIDTH, InventoryMenu::INVENTORY_HEIGHT});
}

IntroductionMenu::IntroductionMenu() : introductionConsole(tcod::Console{INTRO_WIDTH, INTRO_HEIGHT}) {
	tcod::draw_rect(introductionConsole, {0, 0, INTRO_WIDTH, INTRO_HEIGHT}, 0, DARK_RED, DARK_RED);
	std::string introductionText = tcod::stringf(
		R"(They said it was punishment. A sentence earned. A fate sealed.

Beneath the staircase is a place soaked in ash and sorrow.
One step down, and you're no longer among the damned.
You ARE the damned.

This is the Underworld: a shifting maze of darkness, crawling with hateful creatures.

No one ever escapes.
No one ever survives.
And yet...

At the 20th Floor lies the exit. A way out.
Few have seen it. Fewer still believe it exists.

You have nothing left but your will. Your wits.
Descend, fight, die... or do the impossible.

Escape. If you feel like trying.








                [Press any key to continue])");
	tcod::print_rect(introductionConsole, {0, 0, INTRO_WIDTH, INTRO_HEIGHT}, introductionText, WHITE, DARK_RED);
}

void IntroductionMenu::update() {
	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN || engine.lastEventType == SDL_EVENT_KEY_DOWN) {
		closeWithState(Engine::IDLE);
		return;
	}
	engine.gameStatus = Engine::MENU;
}

void IntroductionMenu::render(tcod::Console& mainConsole) {
	tcod::blit(
		mainConsole,
		introductionConsole,
		{engine.CONSOLE_WIDTH / 2 - INTRO_WIDTH / 2, engine.CONSOLE_HEIGHT / 2 - INTRO_HEIGHT / 2},
		{0, 0, INTRO_WIDTH, INTRO_HEIGHT});
}

ControlsMenu::ControlsMenu() : controlsConsole(tcod::Console{CONTROL_WIDTH, CONTROL_HEIGHT}) {
	tcod::draw_rect(controlsConsole, {0, 0, CONTROL_WIDTH, CONTROL_HEIGHT}, 0, BLACK, BLACK);
	std::string controlsText = tcod::stringf(
		R"(                          Controls

Movement / Attack - yuhjklbn (vim keys) or 12346789 (numpad)

Open inventory - i

Rest for a turn - s

Descend the stairs - >

Pick up item at foot - g

Cancel - Esc
		)");
	tcod::print_rect(controlsConsole, {0, 0, CONTROL_WIDTH, CONTROL_HEIGHT}, controlsText, WHITE, BLACK);
}

void ControlsMenu::update() {
	if (engine.lastEventType == SDL_EVENT_MOUSE_BUTTON_DOWN || engine.lastEventType == SDL_EVENT_KEY_DOWN) {
		closeWithState(Engine::IDLE);
		return;
	}
	engine.gameStatus = Engine::MENU;
}

void ControlsMenu::render(tcod::Console& mainConsole) {
	tcod::blit(
		mainConsole,
		controlsConsole,
		{engine.CONSOLE_WIDTH / 2 - CONTROL_WIDTH / 2, engine.CONSOLE_HEIGHT / 2 - CONTROL_HEIGHT / 2},
		{0, 0, CONTROL_WIDTH, CONTROL_HEIGHT},
		0.95F,
		0.8F);
}
