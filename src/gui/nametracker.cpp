#include <cassert>

#include "main.hpp"

static constexpr auto LIGHT_AMBER = tcod::ColorRGB{255, 207, 63};

NameTracker::NameTracker(Random* rng) : rng(rng) {
	callNames.clear();
	defaultNames.clear();
	identifyStatus.clear();

	// TODO: fill

	defaultNames['!']["Potion of Full Healing"] = "Clear Potion";
	defaultNames['!']["Potion of Strength"] = "Bubbly Potion";
	defaultNames['!']["Potion of Protection"] = "Thick potion";
	defaultNames['!']["Potion of Poison"] = "Sticky potion";
	defaultNames['!']["Potion of Amnesia"] = "Smoky potion";
	defaultNames['!']["Potion of Confusion"] = "Glowing potion";
	defaultNames['!']["Potion of Fire"] = "Unstable potion";

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

	descriptions["Potion of Full Healing"] =
		"\nFully heals the user along with curing status effects.\nIf used while HP is full, increases max HP by 10.\n";
	descriptions["Potion of Strength"] = "\nPermenantly increases the user's attack power\n";
	descriptions["Potion of Protection"] =
		"\nPermenantly increases the user's defense power,\nwhile increasing their max HP by 25.\n";
	descriptions["Potion of Poison"] = "\nPermenantly lowers the user's attack power.\n";
	descriptions["Potion of Amnesia"] = "\nLet the user forget what the identified items were.\n";
	descriptions["Potion of Confusion"] = "\nConfuse the user, causing them to go in random directions.\n";
	descriptions["Potion of Fire"] =
		"\nSets the user on fire, which burns for 5HP per turn\nand lasts for 20 turns.\nDrinking any other potion "
		"cures burning.\n";
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
		return "\nThis item is unidentified,\nso it's impossible to tell its nature as of now.\nUsing this item or "
			   "reading a scroll of identify\nwould identify all items of this type.\n";
	return descriptions[std::string(itemActor->name)].c_str();
}
