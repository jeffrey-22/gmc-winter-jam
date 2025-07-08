#include <cstdio>

#include "main.hpp"

static constexpr auto RED = tcod::ColorRGB{255, 0, 0};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

Destructible::Destructible(float maxHp, float defense, const char* corpseName)
	: maxHp(maxHp), hp(maxHp), defense(defense), corpseName(corpseName) {}

// Have an owner take damage (subject to defense), calling die() if necessary
float Destructible::takeDamage(Actor* owner, float damage) {
	damage -= defense;
	if (damage > 0) {
		hp -= damage;
		if (hp <= 0) {
			die(owner);
		}
	} else {
		damage = 0;
	}
	return damage;
}

// Have an owner take true damage (NOT subject to defense), calling die() if necessary
float Destructible::takeTrueDamage(Actor* owner, float trueDamage) {
	if (trueDamage > 0) {
		hp -= trueDamage;
		if (hp <= 0) {
			die(owner);
		}
	} else {
		trueDamage = 0;
	}
	return trueDamage;
}

// Transform the actor into a corpse, changing display and making it non-blocking
void Destructible::die(Actor* owner) {
	owner->ch = '%';
	owner->color = tcod::ColorRGB{191, 0, 0};
	owner->name = corpseName;
	owner->blocks = false;
	// Make sure corpses are drawn before living actors
	engine.sendToBack(owner);
}

// Heal by an amount, up to maxHP, returns actual hp increment
float Destructible::heal(float amount) {
	hp += amount;
	if (hp > maxHp) {
		amount -= hp - maxHp;
		hp = maxHp;
	}
	return amount;
}

// Increase max stats, returns actual hp increment
float Destructible::changeStats(float deltaMaxHp, float deltaDefense) {
	hp += deltaMaxHp;
	maxHp += deltaMaxHp;
	defense += deltaDefense;
	return deltaMaxHp;
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char* corpseName)
	: Destructible(maxHp, defense, corpseName) {}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char* corpseName)
	: Destructible(maxHp, defense, corpseName) {}

void MonsterDestructible::die(Actor* owner) {
	// Transform it into a nasty corpse! It doesn't block, can't be attacked and doesn't move
	engine.gui->message(tcod::stringf("%s is dead.", owner->name));
	Destructible::die(owner);
}

void PlayerDestructible::die(Actor* owner) {
	engine.gui->message(tcod::stringf("You died!\nPlease exit and try again...", owner->name), RED);
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}
