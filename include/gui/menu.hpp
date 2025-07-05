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

   protected:
	tcod::Console inventoryConsole;
	Actor* inventoryOwner;
};

class TilePickMenu : public Menu {
   public:
	enum TilePickRange { IN_LINE_OF_SIGHT };
	TilePickMenu(
		Pickable* invoker,
		Actor* owner,
		Actor* wearer,
		bool allowCancel = true,
		TilePickRange pickRange = IN_LINE_OF_SIGHT);
	~TilePickMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	void handleCancel();

	Pickable* invoker;
	Actor* owner;
	Actor* wearer;
	bool allowCancel;
	TilePickRange pickRange;

	float maxRange = 12.0;
};

class ItemPickMenu : public Menu {
   public:
	ItemPickMenu(Pickable* invoker, Actor* owner, Actor* wearer, bool allowCancel = true);
	~ItemPickMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	void handleCancel();

	tcod::Console itemPickConsole;
	Pickable* invoker;
	Actor* owner;
	Actor* wearer;
	bool allowCancel;
	int invokerShortcutChar;
};

class ItemDropMenu : public Menu {
   public:
	ItemDropMenu(Actor* inventoryOwner);
	~ItemDropMenu() = default;

	virtual void update() override;
	virtual void render(tcod::Console& mainConsole) override;

   protected:
	tcod::Console itemDropConsole;
	Actor* inventoryOwner;
};
