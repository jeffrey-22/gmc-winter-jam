#include "main.hpp"

static auto constexpr LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};
static auto constexpr CYAN = tcod::ColorRGB{0, 255, 255};
static auto constexpr ORANGE = tcod::ColorRGB{255, 127, 0};
static auto constexpr LIGHT_GREEN = tcod::ColorRGB{63, 255, 63};
static auto constexpr LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

TargetSelector::TargetSelector(SelectorType type, float range) : type(type), range(range) {}

Menu* TargetSelector::selectTargets(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	std::vector<Actor*> list = {};
	switch (type) {
		case CLOSEST_MONSTER: {
			Actor* closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
			if (closestMonster) {
				list.push_back(closestMonster);
			} else {
				engine.gui->message("No enemies nearby.", LIGHT_GREY);
			}
			owner->pickable->applyEffects(owner, wearer, false, list);
			return NULL;
		} break;
		case SELECTED_MONSTER: {
			engine.gui->message("Left-click to select an enemy,\nor right-click to cancel.", CYAN);
			return new TilePickMenu(this, owner, wearer, true, TilePickMenu::IN_LINE_OF_SIGHT);
		} break;
		case WEARER_RANGE: {
			for (auto actor : engine.actors) {
				if (actor != wearer && actor->destructible && !actor->destructible->isDead() &&
					actor->getDistance(wearer->x, wearer->y) <= range) {
					list.push_back(actor);
				}
			}
			if (list.empty()) engine.gui->message("No enemies nearby.", LIGHT_GREY);
			owner->pickable->applyEffects(owner, wearer, false, list);
			return NULL;
		} break;
		case SELECTED_RANGE: {
			engine.gui->message(
				tcod::stringf(
					"This item has an impact range of %g tiles.\nLeft-click to select the impact center,\nor "
					"right-click to cancel.",
					range),
				CYAN);
			return new TilePickMenu(this, owner, wearer, true, TilePickMenu::IN_LINE_OF_SIGHT);
		} break;
		case WEARER_HIMSELF: {
			if (wearer) list.push_back(wearer);
			if (list.empty()) engine.gui->message("Cannot find oneself to use on.", LIGHT_GREY);
			owner->pickable->applyEffects(owner, wearer, false, list);
		} break;
		case OTHER_ITEM_FROM_INVENTORY: {
			engine.gui->message("Pick an item from inventory to use on.", CYAN);
			return new ItemPickMenu(this, owner, wearer, true);
		} break;
	}
	return NULL;
}

Menu* TargetSelector::tilePickCallback(
	Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu) {
	std::vector<Actor*> list = {};
	// Allow cancelling, going back to inventory (not spending the turn)
	if (isCancelled) {
		owner->pickable->applyEffects(owner, wearer, true, list);
		return new InventoryMenu(wearer);
	}
	switch (type) {
		case SELECTED_MONSTER: {
			Actor* selectedActor = engine.getActor(x, y);
			if (selectedActor && selectedActor->destructible && !selectedActor->destructible->isDead())
				list.push_back(selectedActor);
			if (list.empty()) engine.gui->message("No enemies selected.", LIGHT_GREY);
			owner->pickable->applyEffects(owner, wearer, false, list);
			return NULL;
		} break;
		case SELECTED_RANGE: {
			for (auto actor : engine.actors) {
				if (actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
					list.push_back(actor);
				}
			}
			if (list.empty()) engine.gui->message("No enemies close enough to the selected center.", LIGHT_GREY);
			owner->pickable->applyEffects(owner, wearer, false, list);
			return NULL;
		} break;
	}
}

Menu* TargetSelector::actorPickCallback(
	Actor* owner, Actor* wearer, bool isCancelled, Actor* selected, Menu* callbackMenu) {
	std::vector<Actor*> list = {};
	// Allow cancelling, going back to inventory (not spending the turn)
	if (isCancelled) {
		owner->pickable->applyEffects(owner, wearer, true, list);
		return new InventoryMenu(wearer);
	}
	switch (type) {
		case OTHER_ITEM_FROM_INVENTORY: {
			if (selected) list.push_back(selected);
			owner->pickable->applyEffects(owner, wearer, false, list);
			return NULL;
		} break;
	}
}
