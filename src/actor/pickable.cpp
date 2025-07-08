#include "main.hpp"

static auto constexpr LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};
static auto constexpr CYAN = tcod::ColorRGB{0, 255, 255};
static auto constexpr ORANGE = tcod::ColorRGB{255, 127, 0};
static auto constexpr LIGHT_GREEN = tcod::ColorRGB{63, 255, 63};
static auto constexpr LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

Pickable::Pickable(TargetSelector* selector, Effect* effect) : selector(selector), effect(effect) {}

Pickable::Pickable() : selector(NULL), effect(NULL) {}

Pickable::~Pickable() {
	if (selector) delete selector;
	if (effect) delete effect;
}

// Put owner into wearer's inventory, now owner is only in container and not in main actor list
// Return if item is successfully put, fails if wearer has no container component or if container is full
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
		if (wearer == engine.player)
			engine.gui->message(tcod::stringf("You drop a %s.", engine.nameTracker->getDisplayName(owner)), LIGHT_GREY);
		else
			engine.gui->message(
				tcod::stringf("%s drops a %s.", wearer->name, engine.nameTracker->getDisplayName(owner)), LIGHT_GREY);
	}
}

void Pickable::swap(Actor* owner, Actor* groundItem, Actor* wearer) {
	if (wearer->container) {
		wearer->container->remove(owner);
		engine.actors.push_back(owner);
		engine.sendToBack(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		if (wearer == engine.player)
			engine.gui->message(
				tcod::stringf("You swap %s\nwith the item on the ground.", engine.nameTracker->getDisplayName(owner)),
				LIGHT_GREY);
		else
			engine.gui->message(
				tcod::stringf(
					"%s swaps %s\nwith the item on the ground.",
					wearer->name,
					engine.nameTracker->getDisplayName(owner)),
				LIGHT_GREY);
		wearer->container->add(groundItem);
		engine.removeActor(groundItem);
	}
}

Menu* Pickable::use(Actor* owner, Actor* wearer, Menu* inventoryMenu) {
	return selector->selectTargets(owner, wearer, inventoryMenu);
}

// Apply effect, then delete item if successful
void Pickable::applyEffects(Actor* owner, Actor* wearer, bool isCancelled, std::vector<Actor*> targets) {
	if (isCancelled) {
		return;
	}
	bool succeed = true;
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

Menu* Pickable::tilePickCallback(Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu) {
	std::runtime_error("Tile pick call back not implemented!");
	return NULL;
}

ScrollOfTeleportationPickable::ScrollOfTeleportationPickable() {
	Pickable::Pickable();
	selector = new TargetSelector(TargetSelector::POSITION_FOR_SELF, 0.0);
}

Menu* ScrollOfTeleportationPickable::tilePickCallback(
	Actor* owner, Actor* wearer, bool isCancelled, int x, int y, Menu* callbackMenu) {
	auto [cx, cy] = engine.map->findSpotsNear(x, y);
	if (cx != -1 && cy != -1) {
		wearer->x = cx;
		wearer->y = cy;
		if (wearer == engine.player) engine.map->computeFov();
		engine.gui->message("You teleported!", LIGHT_GREEN);
	} else {
		engine.gui->message("You failed to teleport...", LIGHT_GREEN);
	}
	if (wearer->container) {
		wearer->container->remove(owner);
		delete owner;
	}
	return NULL;
}
