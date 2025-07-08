#include "main.hpp"

static constexpr auto RED = tcod::ColorRGB{255, 0, 0};
static constexpr auto GOLD = tcod::ColorRGB{229, 191, 0};
static constexpr auto LIGHT_GREY = tcod::ColorRGB{159, 159, 159};
static constexpr auto LIGHT_BLUE = tcod::ColorRGB{63, 63, 255};

SequentialEffect::SequentialEffect(Effect* firstEffect, Effect* secondEffect) {
	memberEffects.clear();
	memberEffects.push_back(firstEffect);
	memberEffects.push_back(secondEffect);
}

bool SequentialEffect::applyTo(Actor* actor) {
	bool atLeastOneSuccess = false;
	for (auto memberEffect : memberEffects) {
		atLeastOneSuccess |= memberEffect->applyTo(actor);
	}
	return atLeastOneSuccess;
}

HealthEffect::HealthEffect(float amount, float maxAmountOnFull, const char* message)
	: amount(amount), maxAmountOnFull(maxAmountOnFull), message(message) {}

bool HealthEffect::applyTo(Actor* actor) {
	if (!actor->destructible) return false;
	if (amount > 0 || (amount >= 0 && maxAmountOnFull > 0)) {
		float pointsHealed = actor->destructible->heal(amount);
		if (pointsHealed > 0) {
			if (message) {
				engine.gui->message(
					tcod::stringf(message, actor == engine.player ? "You" : actor->name, pointsHealed), LIGHT_GREY);
			}
			return true;
		} else {
			float deltaDefense = 0.0F;
			if (maxAmountOnFull >= 24.0F) deltaDefense = 2.0F;
			pointsHealed = actor->destructible->changeStats(maxAmountOnFull, deltaDefense);
			engine.gui->message(
				tcod::stringf(message, actor == engine.player ? "You" : actor->name, pointsHealed), LIGHT_GREY);
			return true;
		}
	} else {
		if (message && -amount - actor->destructible->defense > 0) {
			engine.gui->message(
				tcod::stringf(
					message, actor == engine.player ? "You" : actor->name, -amount - actor->destructible->defense),
				LIGHT_GREY);
		}
		if (actor->destructible->takeDamage(actor, -amount) > 0) {
			return true;
		}
	}
	return false;
}

AiChangeEffect::AiChangeEffect(TemporaryAi* newAi, const char* message) : newAi(newAi), message(message) {}

bool AiChangeEffect::applyTo(Actor* actor) {
	if (typeid(*(actor->ai)).name() == typeid(*(newAi)).name()) {
		engine.gui->message(tcod::stringf(message, actor->name), LIGHT_GREY);
		return true;
	}
	newAi->applyTo(actor);
	if (message) {
		engine.gui->message(tcod::stringf(message, actor->name), LIGHT_GREY);
	}
	return true;
}

PowerChangeEffect::PowerChangeEffect(float deltaPower) : deltaPower(deltaPower) {}

bool PowerChangeEffect::applyTo(Actor* actor) {
	if (actor->attacker) actor->attacker->changePower(actor, deltaPower);
	return true;
}

AmnesiaEffect::AmnesiaEffect() {}

bool AmnesiaEffect::applyTo(Actor* actor) {
	if (actor == engine.player) {
		engine.nameTracker->forgetEverything();
		engine.gui->message("You forgot what the identified items were!", RED);
	}
	return true;
}

SetOnFireEffect::SetOnFireEffect(int nbTurns, float damagePerTurn) : nbTurns(nbTurns), damagePerTurn(damagePerTurn) {}

bool SetOnFireEffect::applyTo(Actor* actor) {
	Actor* fireActor = NULL;
	for (auto existingActor : engine.actors)
		if (FireAi* existingFireAi = dynamic_cast<FireAi*>(existingActor->ai)) {
			if (existingFireAi->target == actor) {
				fireActor = existingActor;
				break;
			}
		}
	if (fireActor == NULL) {
		fireActor = new Actor(-1, -1, 'F', "fire", RED);
		engine.actors.push_back(fireActor);
	}
	// Override its ai
	if (fireActor->ai) delete fireActor->ai;
	if (fireActor->attacker) delete fireActor->attacker;
	fireActor->ai = new FireAi(actor, 20);
	fireActor->attacker = new Attacker(5.0F);
	return true;
}

bool PutOutFireEffect::applyTo(Actor* actor) {
	Actor* fireActor = NULL;
	for (auto existingActor : engine.actors)
		if (FireAi* existingFireAi = dynamic_cast<FireAi*>(existingActor->ai)) {
			if (existingFireAi->target == actor) {
				fireActor = existingActor;
				break;
			}
		}
	if (fireActor != NULL) {
		bool success = false;
		if (fireActor->ai) {
			if (FireAi* fireAi = dynamic_cast<FireAi*>(fireActor->ai)) {
				if (fireAi->nbTurns > 0 && fireAi->target == engine.player) {
					success = true;
				}
			}
		}
		engine.removeActor(fireActor);
		if (success) engine.gui->message("You put out the fire with the liquid in the potion.");
	}
	return true;
}

bool IdentifyEffect::applyTo(Actor* actor) {
	engine.nameTracker->identifyItem(actor);
	return true;
}

bool ConfusionEffect::applyTo(Actor* actor) {
	if (actor == engine.player) {
		engine.gui->message("You feel dizzy...", RED);
		ConfusedPlayerAi* newAi = new ConfusedPlayerAi(12);
		if (typeid(*(actor->ai)).name() == typeid(*(newAi)).name()) {
			return true;
		}
		newAi->applyTo(actor);
		return true;
	} else if (actor->destructible && !actor->destructible->isDead()) {
		engine.gui->message(tcod::stringf("%s appears dizzy...", actor->name), LIGHT_GREY);
		ConfusedMonsterAi* newAi = new ConfusedMonsterAi(12);
		if (typeid(*(actor->ai)).name() == typeid(*(newAi)).name()) {
			return true;
		}
		newAi->applyTo(actor);
		return true;
	}
	return true;
}

bool MappingEffect::applyTo(Actor* actor) {
	engine.map->revealMap();
	engine.gui->message("You suddenly receive a vision of the floor!", LIGHT_BLUE);
	return true;
}

bool LiquifyEffect::applyTo(Actor* actor) {
	if (actor->pickable) {
		Item::setRandomPotion(actor);
		engine.gui->message("The item becomes a potion!");
	}
	return true;
}

bool SummonMonsterEffect::applyTo(Actor* actor) {
	int nbMonsters = 5;
	for (int i = 0; i < nbMonsters; i++) {
		auto [x, y] = engine.map->findSpotsNear(actor->x, actor->y);
		if (x != -1 || y != -1) engine.map->addMonster(x, y);
	}
	engine.gui->message("Monster party!");
	return true;
}
