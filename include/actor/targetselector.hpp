#pragma once

#include "main.hpp"

class TargetSelector {
   public:
	// SELECTED_RANGE:
	enum SelectorType { CLOSEST_MONSTER, SELECTED_MONSTER, WEARER_RANGE, SELECTED_RANGE, WEARER_HIMSELF };
	TargetSelector(SelectorType type, float range);

	// Returns new menu on call, wait for callback in owner's pickable component
	Menu* selectTargets(Actor* owner, Actor* wearer, Menu* inventoryMenu = NULL);

	// Callback method for TilePickMenu. Menu will auto destruct after callback is finished if not handled.
	// If returns NULL, Menu shall self destruct and spend the turn.
	// If returns callbackMenu, Menu shall continue listening for input.
	// If returns another menu pointer, Menu shall close itself and promote that new menu.
	Menu* tilePickCallback(Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu = NULL);
	Menu* actorPickCallback(Actor* owner, Actor* wearer, bool isCancelled, Actor* selected, Menu* callbackMenu = NULL);

   protected:
	SelectorType type;
	float range;
};
