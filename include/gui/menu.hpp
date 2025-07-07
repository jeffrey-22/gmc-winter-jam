#pragma once

#include "main.hpp"

class Menu {
   public:
	Menu(bool setupMenuInEngine = true) {
		if (setupMenuInEngine) {
			engine.gui->menu = this;
			engine.gui->isMenuOpen = true;
			engine.gameStatus = Engine::MENU;
		}
	}

	virtual void update() = 0;
	virtual void render(tcod::Console& mainConsole) = 0;

	void closeWithState(Engine::GameStatus newStatus = Engine::IDLE);
	void closeWithMenuSwitch(Menu* newMenu);

	virtual ~Menu() = default;
};

class InventoryMenu : public Menu {
   public:
	InventoryMenu(Actor* inventoryOwner);
	~InventoryMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

	void useItemAtIndex(int itemIndex);

   protected:
	int selectedIndex;
	tcod::Console inventoryConsole;
	Actor* inventoryOwner;

	bool onCallMenu;
	std::string callString;

	void renderInventory();

	friend class ItemPickMenu;

	static constexpr int INVENTORY_WIDTH = 50;
	static constexpr int INVENTORY_HEIGHT = 38;
};

class TilePickMenu : public Menu {
   public:
	enum TilePickRange { IN_LINE_OF_SIGHT };
	TilePickMenu(
		TargetSelector* invoker,
		Actor* owner,
		Actor* wearer,
		bool allowCancel = true,
		TilePickRange pickRange = IN_LINE_OF_SIGHT);
	~TilePickMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	void handleCancel();

	TargetSelector* invoker;
	Actor* owner;
	Actor* wearer;
	bool allowCancel;
	TilePickRange pickRange;

	float maxRange = 12.0;
};

class ItemPickMenu : public Menu {
   public:
	ItemPickMenu(TargetSelector* invoker, Actor* owner, Actor* wearer, bool allowCancel = true);
	~ItemPickMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	void handleCancel();

	tcod::Console itemPickConsole;
	TargetSelector* invoker;
	Actor* owner;
	Actor* wearer;
	bool allowCancel;
	int invokerShortcutChar;
};

class IntroductionMenu : public Menu {
   public:
	IntroductionMenu();
	~IntroductionMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	tcod::Console introductionConsole;

	static constexpr int INTRO_WIDTH = 60;
	static constexpr int INTRO_HEIGHT = 30;
};

class ControlsMenu : public Menu {
   public:
	ControlsMenu();
	~ControlsMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	tcod::Console controlsConsole;

	static constexpr int CONTROL_WIDTH = 60;
	static constexpr int CONTROL_HEIGHT = 30;
};
