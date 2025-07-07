#pragma once

#include "main.hpp"

// Pickable item that can be picked up and used
class Pickable {
   public:
	Pickable(TargetSelector* selector, Effect* effect);
	Pickable();
	virtual ~Pickable();

	virtual bool pick(Actor* owner, Actor* wearer);
	virtual void drop(Actor* owner, Actor* wearer);
	virtual void swap(Actor* owner, Actor* groundItem, Actor* wearer);

	/*
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

	/*
		Eventually called in the default implementation of use()
		Callback function after using the item.
		Actually applies effects and/or delete the item.
	*/
	virtual void applyEffects(Actor* owner, Actor* wearer, bool isCancelled, std::vector<Actor*> targets);

   protected:
	TargetSelector* selector;
	Effect* effect;
};
