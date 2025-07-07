#include <cassert>
#include <string>

#include "main.hpp"

static const int PANEL_HEIGHT = 8;
static const int BAR_WIDTH = 20;

static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT - 1;

static constexpr auto BLACK = tcod::ColorRGB{0, 0, 0};
static constexpr auto WHITE = tcod::ColorRGB{255, 255, 255};
static constexpr auto LIGHT_RED = tcod::ColorRGB{255, 63, 63};
static constexpr auto DARKER_RED = tcod::ColorRGB{127, 0, 0};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};
static constexpr auto LIGHT_GREEN = tcod::ColorRGB{63, 255, 63};

Gui::Gui() : guiConsole(tcod::Console{engine.MAP_WIDTH, PANEL_HEIGHT}), isMenuOpen(false), menu(NULL) { log.clear(); }

Gui::~Gui() {
	for (auto message : log) delete message;
}

Gui::Message::Message(std::string text, const tcod::ColorRGB& color) : text(text), color(color) {}

Gui::Message::~Message() {}

void Gui::message(std::string text, const tcod::ColorRGB& color) {
	// Split text by newlines
	std::vector<std::string> lines;
	size_t start = 0;
	size_t len = text.length();
	for (size_t i = 0; i < len; ++i) {
		if (text[i] == '\n') {
			lines.emplace_back(text.substr(start, i - start));
			start = i + 1;
		}
	}
	if (start < len) {
		lines.emplace_back(text.substr(start));
	}

	for (auto line : lines) {
		// Make room for the new message
		if (log.size() == MSG_HEIGHT) {
			Message* toRemove = log[0];
			log.erase(log.begin());
			delete toRemove;
		}
		Message* msg = new Message(line, color);
		log.push_back(msg);
	}
}

void Gui::render(tcod::Console& mainConsole) {
	// Clear the GUI console
	guiConsole.clear();
	// Draw the health bar
	renderBar(
		1,
		1,
		BAR_WIDTH,
		"HP",
		engine.player->destructible->hp,
		engine.player->destructible->maxHp,
		LIGHT_GREEN,
		LIGHT_RED,
		DARKER_RED);
	// Draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (auto message : log) {
		auto currentColor = tcod::ColorRGB{TCODColor::lerp(message->color, BLACK, 1.0F - colorCoef)};
		tcod::print(guiConsole, {MSG_X, y}, message->text, currentColor, std::nullopt);
		y++;
		if (colorCoef < 1.0f) {
			colorCoef += 0.3f;
		}
	}
	// Draw actor names selected by mouse, if in FoV
	renderMouseLook();

	// Draw Dungeon Level
	tcod::print(guiConsole, {2, 3}, tcod::stringf("Floor %d", engine.level), WHITE, std::nullopt);

	// Blit GUI console to the main console
	tcod::blit(mainConsole, guiConsole, {0, engine.MAP_HEIGHT}, {0, 0, engine.MAP_WIDTH, PANEL_HEIGHT}, 0.7f, 0.7f);

	// Blit menu console if it is ready
	if (isMenuOpen) {
		menu->render(mainConsole);
	}
}

void Gui::renderBar(
	int x,
	int y,
	int width,
	const char* name,
	float value,
	float maxValue,
	const tcod::ColorRGB& barColorFull,
	const tcod::ColorRGB& barColorEmpty,
	const tcod::ColorRGB& backColor) {
	// Fill the background of the bar
	tcod::draw_rect(guiConsole, {x, y, width, 1}, 0, std::nullopt, backColor);
	float healthRatio = value / maxValue;
	int barWidth = (int)(value / maxValue * width);
	if (value <= 0.0001)
		barWidth = 0;
	else
		barWidth = std::max(barWidth, 1);
	if (barWidth > 0) {
		// Draw the bar
		tcod::ColorRGB barDisplayColor;
		barDisplayColor.r = (uint8_t)(healthRatio * barColorFull.r + (1.0f - healthRatio) * barColorEmpty.r);
		barDisplayColor.g = (uint8_t)(healthRatio * barColorFull.g + (1.0f - healthRatio) * barColorEmpty.g);
		barDisplayColor.b = (uint8_t)(healthRatio * barColorFull.b + (1.0f - healthRatio) * barColorEmpty.b);
		tcod::draw_rect(guiConsole, {x, y, barWidth, 1}, 0, std::nullopt, barDisplayColor);
	}
	// Display HP info text
	std::string hpInfoText = tcod::stringf("%s : %g/%g", name, value, maxValue);
	tcod::print(guiConsole, {x + width / 2, y}, hpInfoText, WHITE, std::nullopt, TCOD_CENTER, TCOD_BKGND_OVERLAY);
}

void Gui::renderMouseLook() {
	int cx = engine.lastMouseTileX, cy = engine.lastMouseTileY;
	if (!engine.map->isInFov(cx, cy)) {
		// If mouse is out of fov, nothing to render
		return;
	}
	std::string allNames = "";
	bool first = true;
	for (auto actor : engine.actors) {
		// Find actors under the mouse cursor
		bool corpseOrEnemyOrItem = (actor->destructible && actor->destructible->isDead()) || actor->pickable ||
								   (actor->destructible && !actor->destructible->isDead() && actor != engine.player);
		if (corpseOrEnemyOrItem && actor->x == cx && actor->y == cy) {
			if (!first) {
				allNames += ", ";
			} else {
				first = false;
			}
			if (actor->pickable)
				allNames += engine.nameTracker->getDisplayName(actor);
			else
				allNames += actor->name;
		}
	}
	// Display the list of actors under the mouse cursor
	tcod::print(guiConsole, {1, 0}, allNames, LIGHT_GREY, std::nullopt);
}

// Open up inventory while changing the state to accept menu inputs. Updates will be informed by calling Gui::update()
// owner is the inventory owner.
void Gui::openInventory(Actor* owner) { new InventoryMenu(owner); }

// Called if in menu and got input, handle state update inside (from MENU_UPDATE to something else)
void Gui::update() {
	if (isMenuOpen) menu->update();
}
