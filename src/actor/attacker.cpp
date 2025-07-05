#include <stdio.h>

#include "main.hpp"

static constexpr auto RED = tcod::ColorRGB{255, 0, 0};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

Attacker::Attacker(float power) : power(power) {}

void Attacker::attack(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		if (power - target->destructible->defense > 0) {
			engine.gui->message(
				tcod::stringf(
					"%s attacks %s for %g hit points.",
					owner->name,
					target->name,
					power - target->destructible->defense),
				owner == engine.player ? RED : LIGHT_GREY);
		} else {
			engine.gui->message(tcod::stringf("%s attacks %s but it has no effect!", owner->name, target->name));
		}
		target->destructible->takeDamage(target, power);
	} else {
		engine.gui->message(tcod::stringf("%s attacks %s in vain.", owner->name, target->name));
	}
}
