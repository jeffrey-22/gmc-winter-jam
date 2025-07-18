#include <cassert>

#include "main.hpp"

static constexpr auto LIGHT_AMBER = tcod::ColorRGB{255, 207, 63};

NameTracker::NameTracker(Random* rng) : rng(rng) {
	callNames.clear();
	identifyStatus.clear();

	initDefaultNames();

	descriptions["Potion of Full Healing"] =
		"\nFully heals the user along with curing status effects.\nIf used while HP is full, increases max HP by 10.\n";
	descriptions["Potion of Strength"] = "\nPermenantly increases the user's attack power\n";
	descriptions["Potion of Protection"] =
		"\nPermenantly increases the user's defense power,\nwhile increasing their max HP by 25.\n";
	descriptions["Potion of Poison"] = "\nPermenantly lowers the user's attack power.\n";
	descriptions["Potion of Amnesia"] = "\nLet the user forget what the identified items were.\n";
	descriptions["Potion of Confusion"] =
		"\nConfuse the user, causing them to go in random directions.\nLasts for 12 turns.\n";
	descriptions["Potion of Fire"] =
		"\nSets the user on fire, which burns for 5HP per turn\nand lasts for 20 turns.\nDrinking any other potion "
		"cures burning.\n";

	descriptions["Scroll of Identify"] = "\nIdentify the selected item.\n";
	descriptions["Scroll of Teleportation"] = "\nTeleport the reader to a location in their sight.\n";
	descriptions["Scroll of Mapping"] = "\nReveal the floor map as well as locations of monsters and items.\n";
	descriptions["Scroll of Confusion"] = "\nConfuse all monsters in the same room or 1 tile around you.\n";
	descriptions["Scroll of Fireball"] =
		"\nIgnite anyone within a 3x3 impact range at the desired location.\nBurning deals 5HP per turn and lasts 20 "
		"turns.\n";
	descriptions["Scroll of Liquify"] = "\nChange a selected item into a random potion.\n";
	descriptions["Scroll of Summon Monsters"] = "\nSummon monsters around the reader's location.\n";
}

void NameTracker::initDefaultNames() {
	defaultNames.clear();

	defaultNames['!']["Potion of Full Healing"] = "Clear Potion";
	defaultNames['!']["Potion of Strength"] = "Bubbly Potion";
	defaultNames['!']["Potion of Protection"] = "Thick Potion";
	defaultNames['!']["Potion of Poison"] = "Sticky Potion";
	defaultNames['!']["Potion of Amnesia"] = "Smoky Potion";
	defaultNames['!']["Potion of Confusion"] = "Glowing Potion";
	defaultNames['!']["Potion of Fire"] = "Unstable Potion";

	defaultNames['?']["Scroll of Identify"] = "Azure Scroll";
	defaultNames['?']["Scroll of Teleportation"] = "Crimson Scroll";
	defaultNames['?']["Scroll of Mapping"] = "Verdant Scroll";
	defaultNames['?']["Scroll of Confusion"] = "Saffron Scroll";
	defaultNames['?']["Scroll of Fireball"] = "Violet Scroll";
	defaultNames['?']["Scroll of Liquify"] = "Ebon Scroll";
	defaultNames['?']["Scroll of Summon Monsters"] = "Ivory Scroll";

	// Randomly shuffle names
	for (auto [typeCh, typeDefaultNames] : defaultNames) {
		std::vector<std::string> names;
		for (const auto& pair : typeDefaultNames) {
			names.push_back(pair.second);
			assert((int)(pair.second.size()) <= MAX_CALL_STRING_LENGTH);
		}
		for (int i = 1; i < names.size(); i++) std::swap(names[i], names[rng->getBoundedInt(0, i)]);
		int nameIndex = 0;
		for (auto& pair : typeDefaultNames) {
			defaultNames[typeCh][pair.first] = names[nameIndex++];
		}
	}
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

void NameTracker::forgetEverything() {
	callNames.clear();
	identifyStatus.clear();

	initDefaultNames();
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
