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
		case ROOM_OR_AROUND: {
			// room or around, excluding wearer
			int x1, y1, x2, y2, cx, cy;
			cx = wearer->x, cy = wearer->y;
			bool foundRoom = false;
			for (auto [roomx1, roomy1, roomx2, roomy2] : engine.map->roomRecords) {
				if (roomx1 <= cx && cx <= roomx2 && roomy1 <= cy && cy <= roomy2) {
					foundRoom = true;
					x1 = roomx1;
					y1 = roomy1;
					x2 = roomx2;
					y2 = roomy2;
					break;
				}
			}
			if (!foundRoom) {
				x1 = cx - 1;
				y1 = cy - 1;
				x2 = cx + 1;
				y2 = cy + 1;
			} else {
				x1--;
				y1--;
				x2++;
				y2++;
			}
			for (auto actor : engine.actors)
				if (actor != wearer && actor->destructible && !actor->destructible->isDead() &&
					(x1 <= actor->x && actor->x <= x2 && y1 <= actor->y && actor->y <= y2)) {
					list.push_back(actor);
				}
			if (list.empty()) engine.gui->message("No enemies nearby.", LIGHT_GREY);
			owner->pickable->applyEffects(owner, wearer, false, list);
		} break;
		case POSITION_FOR_SELF: {
			engine.gui->message("Pick a position that you can see.", CYAN);
			return new TilePickMenu(this, owner, wearer, true, TilePickMenu::IN_LINE_OF_SIGHT);
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
		case POSITION_FOR_SELF: {
			return owner->pickable->tilePickCallback(owner, wearer, isCancelled, x, y, callbackMenu);
		} break;
	}
	return NULL;
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
	return NULL;
}
