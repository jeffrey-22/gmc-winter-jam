#include <stdio.h>

#include "main.hpp"

static constexpr auto RED = tcod::ColorRGB{255, 0, 0};
static constexpr auto GOLD = tcod::ColorRGB{229, 191, 0};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

Attacker::Attacker(float power) : power(power) {}

void Attacker::attack(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		if (power - target->destructible->defense > 0) {
			engine.gui->message(
				tcod::stringf(
					"%s attack%s %s for %g HP.",
					owner == engine.player ? "You" : owner->name,
					owner == engine.player ? "" : "s",
					target == engine.player ? "you" : target->name,
					power - target->destructible->defense),
				target == engine.player ? RED : GOLD);
		} else {
			engine.gui->message(
				tcod::stringf(
					"%s attack%s %s but it has no effect!",
					owner == engine.player ? "You" : owner->name,
					owner == engine.player ? "" : "s",
					target == engine.player ? "you" : target->name));
		}
		target->destructible->takeDamage(target, power);
	} else {
		engine.gui->message(tcod::stringf("%s attacks %s in vain.", owner->name, target->name));
	}
}

void Attacker::burn(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		engine.gui->message(
			tcod::stringf(
				"%s get%s burned for %g HP.",
				target == engine.player ? "You" : target->name,
				target == engine.player ? "" : "s",
				power),
			target == engine.player ? RED : GOLD);
		target->destructible->takeTrueDamage(target, power);
	}
}

void Attacker::changePower(Actor* owner, float deltaPower) {
	if (owner->attacker && owner->destructible && !owner->destructible->isDead()) {
		float originalPower = owner->attacker->power;
		owner->attacker->power += deltaPower;
		owner->attacker->power = std::max(owner->attacker->power, 1.0F);
		float realChanged = owner->attacker->power - originalPower;
		if (realChanged > 0) {
			engine.gui->message(
				tcod::stringf(
					"%s become%s more powerful!",
					owner == engine.player ? "You" : owner->name,
					owner == engine.player ? "" : "s"));
		} else if (realChanged < 0) {
			engine.gui->message(
				tcod::stringf(
					"%s become%s weaker!",
					owner == engine.player ? "You" : owner->name,
					owner == engine.player ? "" : "s"));
		}
	}
}
