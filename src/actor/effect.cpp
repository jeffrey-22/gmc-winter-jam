#include "main.hpp"

static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};

HealthEffect::HealthEffect(float amount, const char* message) : amount(amount), message(message) {}

bool HealthEffect::applyTo(Actor* actor) {
	if (!actor->destructible) return false;
	if (amount > 0) {
		float pointsHealed = actor->destructible->heal(amount);
		if (pointsHealed > 0) {
			if (message) {
				engine.gui->message(tcod::stringf(message, actor->name, pointsHealed), LIGHT_GREY);
			}
			return true;
		}
	} else {
		if (message && -amount - actor->destructible->defense > 0) {
			engine.gui->message(
				tcod::stringf(message, actor->name, -amount - actor->destructible->defense), LIGHT_GREY);
		}
		if (actor->destructible->takeDamage(actor, -amount) > 0) {
			return true;
		}
	}
	return false;
}

AiChangeEffect::AiChangeEffect(TemporaryAi* newAi, const char* message) : newAi(newAi), message(message) {}

bool AiChangeEffect::applyTo(Actor* actor) {
	newAi->applyTo(actor);
	if (message) {
		engine.gui->message(tcod::stringf(message, actor->name), LIGHT_GREY);
	}
	return true;
}
