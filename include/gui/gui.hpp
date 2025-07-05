#pragma once

#include "main.hpp"

class Menu;

class Gui {
   private:
	static constexpr auto DEFAULT_MESSAGE_COLOR_LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

   public:
	Gui();
	~Gui();
	void render(tcod::Console& mainConsole);
	void message(std::string text, const tcod::ColorRGB& color = DEFAULT_MESSAGE_COLOR_LIGHT_GREY);
	void openInventory(Actor* owner);
	void update();

	friend class Menu;
	bool isMenuOpen;
	Menu* menu;

   protected:
	tcod::Console guiConsole;

	struct Message {
		const std::string text;
		const tcod::ColorRGB& color;
		Message(std::string text, const tcod::ColorRGB& color);
		~Message();
	};
	std::vector<Message*> log;

	void renderBar(
		int x,
		int y,
		int width,
		const char* name,
		float value,
		float maxValue,
		const tcod::ColorRGB& barColor,
		const tcod::ColorRGB& backColor);

	void renderMouseLook();
};
