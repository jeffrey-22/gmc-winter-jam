#include "main.hpp"

static auto constexpr LIGHT_YELLOW = tcod::ColorRGB{255, 255, 63};
static auto constexpr VIOLET = tcod::ColorRGB{127, 0, 255};

void Item::addPotionOfFullHealing(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Full Healing", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(
			new PutOutFireEffect(), new HealthEffect(800, 10, "%s used the healing potion and recovered %g HP!")));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfStrength(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Strength", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new PowerChangeEffect(10)));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfProtection(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Protection", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(
			new PutOutFireEffect(), new HealthEffect(0, 25, "%s used the potion and increased %g max HP!")));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfPoison(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Poison", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new PowerChangeEffect(-15)));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfAmnesia(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Amnesia", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(new PutOutFireEffect(), new AmnesiaEffect()));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfConfusion(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Confusion", VIOLET);
	potion->blocks = false;
	potion->pickable = new Pickable(
		new TargetSelector(TargetSelector::WEARER_HIMSELF, 0),
		new SequentialEffect(
			new PutOutFireEffect(),
			new AiChangeEffect(new ConfusedPlayerAi(12), "You used the confusion potion.\nYou felt dizzy...")));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addPotionOfFire(int x, int y) {
	Actor* potion = new Actor(x, y, '!', "Potion of Fire", VIOLET);
	potion->blocks = false;
	potion->pickable =
		new Pickable(new TargetSelector(TargetSelector::WEARER_HIMSELF, 0), new SetOnFireEffect(20, 5.0F));
	engine.actors.push_back(potion);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(potion);
}

void Item::addScrollOfIdentify(int x, int y) {
	Actor* scroll = new Actor(x, y, '?', "Scroll of Identify", LIGHT_YELLOW);
	scroll->blocks = false;
	// scroll->pickable =
	// 	new Pickable(new TargetSelector(TargetSelector::WEARER_HIMSELF, 0), );
	engine.actors.push_back(scroll);
	// Items visuals should be at the back to not hide actors standing over them
	engine.sendToBack(scroll);
}
