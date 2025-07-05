#include <cassert>

#include "main.hpp"

Container::Container(int size) : size(size) { inventory.clear(); }

Container::~Container() {
	for (auto actor : inventory) delete actor;
}

// Add an actor to container, return if successfully added. Fail if inventory is full
bool Container::add(Actor* actor) {
	if (size > 0 && inventory.size() >= size) {
		// inventory full
		return false;
	}
	inventory.push_back(actor);
	return true;
}

// Check if actor is in inventory
bool Container::contains(Actor* actor) {
	auto it = std::find(inventory.begin(), inventory.end(), actor);
	return (it != inventory.end());
}

void Container::remove(Actor* actor) {
	auto it = std::find(inventory.begin(), inventory.end(), actor);
	assert(it != inventory.end());
	inventory.erase(it);
}

// Use an inventory item by calling its pickable use()
// Completely delete the actor and remove it from wearer's inventory, releasing its memory
// Return if item is successfully found and deleted
bool Container::use(Actor* actor, Actor* wearer) {
	if (actor->pickable && actor->pickable->use(actor, wearer)) return true;
	return false;
}

bool Container::isIndexValid(int index) { return index >= 0 && index < inventory.size(); }
