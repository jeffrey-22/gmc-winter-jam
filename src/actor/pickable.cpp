#include "main.hpp"

static auto constexpr LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};
static auto constexpr CYAN = tcod::ColorRGB{0, 255, 255};
static auto constexpr ORANGE = tcod::ColorRGB{255, 127, 0};
static auto constexpr LIGHT_GREEN = tcod::ColorRGB{63, 255, 63};
static auto constexpr LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

// Put owner into wearer's inventory, now owner is only in container and not in main actor list
// Return if item is successfully put
bool Pickable::pick(Actor* owner, Actor* wearer) {
	if (wearer->container && wearer->container->add(owner)) {
		engine.removeActor(owner);
		return true;
	}
	return false;
}

// Completely delete the actor and remove it from wearer's inventory, releasing its memory
// Return NULL if successful, and inventoryMenu if not
Menu* Pickable::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	if (wearer->container) {
		wearer->container->remove(owner);
		delete owner;
		return NULL;
	}
	return inventoryMenu;
}

void Pickable::drop(Actor* owner, Actor* wearer) {
	if (wearer->container) {
		wearer->container->remove(owner);
		engine.actors.push_back(owner);
		engine.sendToBack(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		engine.gui->message(tcod::stringf("%s drops a %s.", wearer->name, owner->name), LIGHT_GREY);
	}
}

Healer::Healer(float amount) : amount(amount) {}

Menu* Healer::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	if (wearer->destructible) {
		float amountHealed = wearer->destructible->heal(amount);
		if (amountHealed > 0) {
			engine.gui->message(tcod::stringf("You healed %d HP!", static_cast<int>(amountHealed + 0.5)));
			return Pickable::use(owner, wearer);
		} else {
			engine.gui->message(tcod::stringf("The potion resisted your attempt to drink it!"));
		}
	}
	return inventoryMenu;
}

LightningBolt::LightningBolt(float range, float damage) : range(range), damage(damage) {}

Menu* LightningBolt::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	Actor* closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
	if (!closestMonster) {
		engine.gui->message("No enemy is close enough to strike.");
		return inventoryMenu;
	}
	// hit closest monster for <damage> hit points
	engine.gui->message(
		tcod::stringf(
			"A lighting bolt strikes the %s with a loud thunder!\n"
			"The damage is %g hit points.",
			closestMonster->name,
			damage),
		LIGHT_BLUE);
	closestMonster->destructible->takeDamage(closestMonster, damage);
	return Pickable::use(owner, wearer);
}

Fireball::Fireball(float range, float damage) : LightningBolt(range, damage) {}

Menu* Fireball::use(Actor* owner, Actor* wearer, Menu* InventoryMenu) {
	engine.gui->message("Left-click a target tile for the fireball,\nor right-click to cancel.", CYAN);
	return new TilePickMenu(this, owner, wearer, true, TilePickMenu::IN_LINE_OF_SIGHT);
}

Menu* Fireball::tilePickCallback(Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu) {
	if (isCancelled) {
		return new InventoryMenu(wearer);
	}
	// Burn everything in range (including player)
	engine.gui->message(tcod::stringf("The fireball explodes, burning everything within %g tiles!", range), ORANGE);
	for (auto actor : engine.actors) {
		if (actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
			engine.gui->message(tcod::stringf("The %s gets burned for %g hit points.", actor->name, damage), ORANGE);
			actor->destructible->takeDamage(actor, damage);
		}
	}
	return Pickable::use(owner, wearer);
};

Confuser::Confuser(int nbTurns, float range) : nbTurns(nbTurns), range(range) {}

Menu* Confuser::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	engine.gui->message("Left-click an enemy to confuse it,\nor right-click to cancel.", CYAN);
	return new TilePickMenu(this, owner, wearer, true, TilePickMenu::IN_LINE_OF_SIGHT);
}

Menu* Confuser::tilePickCallback(Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu) {
	if (isCancelled) {
		return new InventoryMenu(wearer);
	}
	Actor* actor = engine.getActor(x, y);
	if (!actor) {
		engine.gui->message(
			"There appears to be no one at that position.\nThe scroll glows and disappears.", LIGHT_GREEN);
		return NULL;
	}
	// confuse the monster for <nbTurns> turns
	Ai* confusedAi = new ConfusedMonsterAi(nbTurns, actor->ai);
	actor->ai = confusedAi;
	engine.gui->message(
		tcod::stringf("The eyes of the %s look vacant,\nas he starts to stumble around!", actor->name), LIGHT_GREEN);
	return Pickable::use(owner, wearer);
};
