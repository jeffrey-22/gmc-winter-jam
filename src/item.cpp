#include "main.hpp"

static auto constexpr LIGHT_YELLOW = tcod::ColorRGB{255, 255, 63};
static auto constexpr VIOLET = tcod::ColorRGB{127, 0, 255};

Actor* Item::newItem(int x, int y) {
	Actor* item = new Actor(x, y, '!', "ITEM!", VIOLET);
	item->blocks = false;
	engine.actors.push_back(item);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(item);
	return item;
}

void Item::setRandomItem(Actor* item) {
	int dice = Random::instance().getInt(0, 100);
	if (dice <= 40) {
		setRandomPotion(item);
	} else {
		setRandomScroll(item);
	}
}

void Item::setRandomPotion(Actor* item) {
	int potionIndex = Random::instance().getInt(0, 10);
	switch (potionIndex) {
		case 0:
		case 7:
		case 8:
		case 9:
			setPotionOfFullHealing(item);
			break;
		case 1:
			setPotionOfStrength(item);
			break;
		case 2:
		case 10:
			setPotionOfProtection(item);
			break;
		case 3:
			setPotionOfPoison(item);
			break;
		case 4:
			setPotionOfAmnesia(item);
			break;
		case 5:
			setPotionOfConfusion(item);
			break;
		case 6:
			setPotionOfFire(item);
			break;
	}
}

void Item::setRandomScroll(Actor* item) {
	int scrollIndex = Random::instance().getInt(0, 10);
	switch (scrollIndex) {
		case 0:
			setScrollOfIdentify(item);
			break;
		case 1:
		case 7:
		case 9:
			setScrollOfTeleportation(item);
			break;
		case 2:
			setScrollOfMapping(item);
			break;
		case 3:
		case 10:
			setScrollOfConfusion(item);
			break;
		case 4:
			setScrollOfFireball(item);
			break;
		case 5:
		case 8:
			setScrollOfLiquify(item);
			break;
		case 6:
			setScrollOfSummonMonsters(item);
			break;
	}
}

void Item::setPotionOfFullHealing(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Full Healing";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(
			new PutOutFireEffect(), new HealthEffect(800, 10, "%s used the healing potion and recovered %g HP!")));
}

void Item::setPotionOfStrength(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Strength";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new PowerChangeEffect(10)));
}

void Item::setPotionOfProtection(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Protection";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(
			new PutOutFireEffect(), new HealthEffect(0, 25, "%s used the potion and increased %g max HP!")));
}

void Item::setPotionOfPoison(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Poison";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new PowerChangeEffect(-15)));
}

void Item::setPotionOfAmnesia(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Amnesia";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new AmnesiaEffect()));
}

void Item::setPotionOfConfusion(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Confusion";
	item->color = VIOLET;
	item->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new ConfusionEffect()));
}

void Item::setPotionOfFire(Actor* item) {
	item->ch = '!';
	item->name = "Potion of Fire";
	item->color = VIOLET;
	item->pickable = new Pickable(new TargetSelector(TargetSelector::WEARER_HIMSELF, 0), new SetOnFireEffect(20, 5.0F));
}

void Item::setScrollOfIdentify(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Identify";
	item->color = LIGHT_YELLOW;
	item->pickable =
		new Pickable(new TargetSelector(TargetSelector::OTHER_ITEM_FROM_INVENTORY, 0), new IdentifyEffect());
}

void Item::setScrollOfTeleportation(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Teleportation";
	item->color = LIGHT_YELLOW;
	item->pickable = new ScrollOfTeleportationPickable();
}

void Item::setScrollOfMapping(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Mapping";
	item->color = LIGHT_YELLOW;
	item->pickable = new Pickable(new TargetSelector(TargetSelector::WEARER_HIMSELF, 0), new MappingEffect());
}

void Item::setScrollOfConfusion(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Confusion";
	item->color = LIGHT_YELLOW;
	item->pickable = new Pickable(new TargetSelector(TargetSelector::ROOM_OR_AROUND, 0), new ConfusionEffect());
}

void Item::setScrollOfFireball(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Fireball";
	item->color = LIGHT_YELLOW;
	item->pickable =
		new Pickable(new TargetSelector(TargetSelector::SELECTED_RANGE, 1.5F), new SetOnFireEffect(10, 10.0F));
}

void Item::setScrollOfLiquify(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Liquify";
	item->color = LIGHT_YELLOW;
	item->pickable =
		new Pickable(new TargetSelector(TargetSelector::OTHER_ITEM_FROM_INVENTORY, 0), new LiquifyEffect());
}

void Item::setScrollOfSummonMonsters(Actor* item) {
	item->ch = '?';
	item->name = "Scroll of Summon Monsters";
	item->color = LIGHT_YELLOW;
	item->pickable = new Pickable(new TargetSelector(TargetSelector::WEARER_HIMSELF, 0), new SummonMonsterEffect());
}
