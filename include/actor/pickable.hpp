#pragma once

#include "main.hpp"

// Pickable item that can be picked up and used
class Pickable {
   public:
	virtual ~Pickable() {};

	virtual bool pick(Actor* owner, Actor* wearer);
	virtual void drop(Actor* owner, Actor* wearer);

	/*
		Use function must be inherited with its own implementation, the default simply
		delete the actor and remove it from wearer's inventory, releasing its memory,
		Returning if item is successfully found and deleted.

		Owner is the pickable item, wearer is the user of the item.
		If called from inventory, its pointer is provided, otherwise inventoryMenu will be NULL.

		If a new menu prompt is needed - return that new menu.
		Inventory menu will close itself and open that new menu.

		If the new menu supports going back to inventory, use callback in that new menu
		where if it gets cancelled, close that prompt menu and reopen inventory.

		If returns NULL, use proceeds normally - a turn is spent and item effect has taken place.

		If returns the inventory pointer, the use action is cancelled and inventory persists.
	*/
	virtual Menu* use(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL);

	// Callback method for TilePickMenu. Menu will auto destruct after callback is finished if not handled.
	// If returns NULL, Menu shall self destruct and spend the turn.
	// If returns callbackMenu, Menu shall continue listening for input.
	// If returns another menu pointer, Menu shall close itself and promote that new menu.
	virtual Menu* tilePickCallback(
		Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu = NULL) {
		throw std::runtime_error("tilePickCallback() not implemented in derived class but called");
	};
	virtual Menu* actorPickCallback(
		Actor* owner, Actor* wearer, bool isCancelled, Actor* selected, Menu* callbackMenu = NULL) {
		throw std::runtime_error("actorPickCallback() not implemented in derived class but called");
	};
};

class Healer : public Pickable {
   public:
	float amount;  // heal by how many hp

	Healer(float amount);
	virtual Menu* use(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL) override;
};

class LightningBolt : public Pickable {
   public:
	float range, damage;
	LightningBolt(float range, float damage);
	virtual Menu* use(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL) override;
};

class Fireball : public LightningBolt {
   public:
	Fireball(float range, float damage);
	virtual Menu* use(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL) override;
	virtual Menu* tilePickCallback(
		Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu = NULL) override;
};

class Confuser : public Pickable {
   public:
	int nbTurns;
	float range;
	Confuser(int nbTurns, float range);
	virtual Menu* use(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL) override;
	virtual Menu* tilePickCallback(
		Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu = NULL) override;
};
