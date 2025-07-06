#include "main.hpp"

static auto constexpr LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};
static auto constexpr CYAN = tcod::ColorRGB{0, 255, 255};
static auto constexpr ORANGE = tcod::ColorRGB{255, 127, 0};
static auto constexpr LIGHT_GREEN = tcod::ColorRGB{63, 255, 63};
static auto constexpr LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

Pickable::Pickable(TargetSelector* selector, Effect* effect) : selector(selector), effect(effect) {}

Pickable::~Pickable() {
	if (selector) delete selector;
	if (effect) delete effect;
}

// Put owner into wearer's inventory, now owner is only in container and not in main actor list
// Return if item is successfully put
bool Pickable::pick(Actor* owner, Actor* wearer) {
	if (wearer->container && wearer->container->add(owner)) {
		engine.removeActor(owner);
		return true;
	}
	return false;
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

Menu* Pickable::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	return selector->selectTargets(owner, wearer, inventoryMenu);
}

void Pickable::applyEffects(Actor* owner, Actor* wearer, bool isCancelled, std::vector<Actor*> targets) {
	if (isCancelled) {
		return;
	}
	bool succeed = false;
	for (auto actor : targets) {
		if (effect->applyTo(actor)) {
			succeed = true;
		}
	}
	// In case of success, delete the item from inventory
	if (succeed) {
		if (wearer->container) {
			wearer->container->remove(owner);
			delete owner;
		}
	}
}
