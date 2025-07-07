#include <cassert>

#include "main.hpp"

static constexpr auto LIGHT_AMBER = tcod::ColorRGB{255, 207, 63};

NameTracker::NameTracker(Random* rng) : rng(rng) {
	callNames.clear();
	defaultNames.clear();
	identifyStatus.clear();

	// TODO: fill

	defaultNames['!']["health potion"] = "? bubbly potion";
	defaultNames['?']["scroll of lightning bolt"] = "? green scroll";
	defaultNames['?']["scroll of confusion"] = "? yellow scroll";
	defaultNames['?']["scroll of fireball"] = "? red scroll";

	// Randomly shuffle names
	for (auto [typeCh, typeDefaultNames] : defaultNames) {
		std::vector<std::string> names;
		for (const auto& pair : typeDefaultNames) {
			names.push_back(pair.second);
			assert((int)(pair.second.size()) <= MAX_CALL_STRING_LENGTH);
		}
		std::shuffle(names.begin(), names.end(), rng->rng);
		int nameIndex = 0;
		for (auto& pair : typeDefaultNames) {
			pair.second = names[nameIndex++];
		}
	}
	descriptions["health potion"] = "\nHeals the user by 4 HP!\n";
}

NameTracker::~NameTracker() { delete rng; }

void NameTracker::callItem(Actor* itemActor, std::string callName) {
	callNames[std::string(itemActor->name)] = callName;
}

void NameTracker::identifyItem(Actor* itemActor) {
	if (identifyStatus[std::string(itemActor->name)]) return;
	engine.gui->message(tcod::stringf("%s is actually %s!", getDisplayName(itemActor), itemActor->name), LIGHT_AMBER);
	identifyStatus[std::string(itemActor->name)] = true;
	// Clears call names for them
	callNames.erase(std::string(itemActor->name));
}

const char* NameTracker::getDisplayName(const Actor* itemActor) {
	if (!itemActor->pickable) {
		return itemActor->name;
	}
	if (callNames.count(std::string(itemActor->name))) return callNames[std::string(itemActor->name)].c_str();
	if (!identifyStatus[std::string(itemActor->name)])
		return defaultNames[itemActor->ch][std::string(itemActor->name)].c_str();
	return itemActor->name;
}

const char* NameTracker::getDescription(const Actor* itemActor) {
	if (!identifyStatus[std::string(itemActor->name)])
		return "\nThis item is unidentified,\nso it's impossible to tell its nature.\nUsing this item or "
			   "a scroll of identify\nidentifies all items of this type.\n";
	return descriptions[std::string(itemActor->name)].c_str();
}
