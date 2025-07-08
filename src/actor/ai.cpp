#include <cassert>

#include "main.hpp"

void PlayerAi::parseInput(
	int& dx,
	int& dy,
	bool& isActionPickUp,
	bool& isActionInventory,
	bool& isActionControlsMenu,
	bool& isActionDescend,
	bool& isActionRest) {
	dx = 0, dy = 0;
	isActionPickUp = false;
	isActionInventory = false;
	isActionControlsMenu = false;
	isActionDescend = false;
	isActionRest = false;
	switch (engine.lastKeyboardEvent.key) {
		case SDLK_LEFT:
		case SDLK_H:
		case SDLK_KP_4:
			dx = -1;
			break;
		case SDLK_RIGHT:
		case SDLK_L:
		case SDLK_KP_6:
			dx = 1;
			break;
		case SDLK_UP:
		case SDLK_K:
		case SDLK_KP_8:
			dy = -1;
			break;
		case SDLK_DOWN:
		case SDLK_J:
		case SDLK_KP_2:
			dy = 1;
			break;
		case SDLK_HOME:
		case SDLK_Y:
		case SDLK_KP_7:
			dx = -1;
			dy = -1;
			break;
		case SDLK_PAGEUP:
		case SDLK_U:
		case SDLK_KP_9:
			dx = 1;
			dy = -1;
			break;
		case SDLK_END:
		case SDLK_B:
		case SDLK_KP_1:
			dx = -1;
			dy = 1;
			break;
		case SDLK_PAGEDOWN:
		case SDLK_N:
		case SDLK_KP_3:
			dx = 1;
			dy = 1;
			break;
		case SDLK_S:
			isActionRest = true;
			break;
		case SDLK_G:
			isActionPickUp = true;
			break;
		case SDLK_I:
			isActionInventory = true;
			break;
		case SDLK_SLASH:
		case SDLK_QUESTION:
			isActionControlsMenu = true;
			break;
		case SDLK_GREATER:
		case SDLK_PERIOD:
			isActionDescend = true;
			break;
		default:
			break;
	}
}

void PlayerAi::update(Actor* owner) {
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Turn not spent, keep IDLEing for effective inputs
		engine.gameStatus = Engine::IDLE;
		return;
	}
	int dx, dy;
	bool isActionPickUp, isActionInventory, isActionControlsMenu, isActionDescend, isActionRest;
	PlayerAi::parseInput(
		dx, dy, isActionPickUp, isActionInventory, isActionControlsMenu, isActionDescend, isActionRest);
	// If turn is spent update engine gamestatus to OTHER_ACTORS_TURN, else return to IDLE
	bool isTurnSpent = false;
	if (dx != 0 || dy != 0) {
		if (moveOrAttack(owner, owner->x + dx, owner->y + dy)) {
			engine.map->computeFov();
			isTurnSpent = true;
		}
	} else if (isActionPickUp) {
		bool foundItem = false;
		for (auto actor : engine.actors)
			if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
				if (actor->pickable->pick(actor, owner)) {
					foundItem = true;
					isTurnSpent = true;
					engine.gui->message(tcod::stringf("You picked up %s.", engine.nameTracker->getDisplayName(actor)));
					break;
				} else {
					engine.gui->message(tcod::stringf("Your inventory is full!"));
					break;
				}
			}
		if (!foundItem) {
			engine.gui->message(tcod::stringf("There's nothing to pick up here."));
		}
	} else if (isActionInventory) {
		openInventory(owner);
		// Game status will be updated after inventory opening is complete
		// So we could return here and not update game state
		return;
	} else if (isActionControlsMenu) {
		new ControlsMenu();
		return;
	} else if (isActionDescend) {
		if (std::tie(owner->x, owner->y) == std::tie(engine.stairs->x, engine.stairs->y)) {
			engine.nextLevel();
		} else {
			engine.gui->message(
				"There are no stairs here.\nDescend by pressing > while standing\ndirectly on top of stairs.");
		}
	} else if (isActionRest) {
		isTurnSpent = true;
	}
	if (isTurnSpent)
		engine.gameStatus = Engine::OTHER_ACTORS_TURN;
	else
		engine.gameStatus = Engine::IDLE;
}

// Returns if turn is spent
bool PlayerAi::moveOrAttack(Actor* owner, int targetx, int targety) {
	if (!engine.map->isWalkable(targetx, targety)) return false;
	// look for other living actors to attack
	for (auto actor : engine.actors) {
		if (actor->destructible && !actor->destructible->isDead() && actor->x == targetx && actor->y == targety) {
			owner->attacker->attack(owner, actor);
			return true;
		}
	}
	// look for corpses and items
	for (auto actor : engine.actors) {
		if (actor->x != targetx || actor->y != targety) continue;
		if (actor->destructible && actor->destructible->isDead()) {
			engine.gui->message(tcod::stringf("There's a %s here\n", actor->name));
		} else if (actor->pickable) {
			if (actor->pickable->pick(actor, owner)) {
				engine.gui->message(tcod::stringf("You picked up the %s.", engine.nameTracker->getDisplayName(actor)));
			} else {
				engine.gui->message(
					tcod::stringf(
						"Your inventory is full, so you walked over\nthe %s.",
						engine.nameTracker->getDisplayName(actor)));
			}
		}
	}
	owner->x = targetx;
	owner->y = targety;
	return true;
}

// Have Gui open inventory, and it will update game state and handle inventory controls
void PlayerAi::openInventory(Actor* owner) {
	assert(owner->container != NULL);
	engine.gui->openInventory(owner);
}

void MonsterAi::update(Actor* owner) {
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if (engine.map->isInFov(owner->x, owner->y)) {
		// we can see the player. move towards him
		moveCount = TRACKING_TURNS;
	} else {
		moveCount--;
	}
	if (moveCount > 0) {
		moveOrAttack(owner, engine.player->x, engine.player->y);
	}
}

void MonsterAi::moveOrAttack(Actor* owner, int targetx, int targety) {
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1 : -1);
	int stepdy = (dy > 0 ? 1 : -1);
	float distance = sqrtf(1.0f * (dx * dx + dy * dy));
	if (distance >= 2) {
		dx = (int)(round(dx / distance));
		dy = (int)(round(dy / distance));
		if (engine.map->canWalk(owner->x + dx, owner->y + dy)) {
			owner->x += dx;
			owner->y += dy;
		} else if (engine.map->canWalk(owner->x + stepdx, owner->y)) {
			owner->x += stepdx;
		} else if (engine.map->canWalk(owner->x, owner->y + stepdy)) {
			owner->y += stepdy;
		}
	} else if (owner->attacker) {
		owner->attacker->attack(owner, engine.player);
	}
}

TemporaryAi::TemporaryAi(int nbTurns) : nbTurns(nbTurns) {}

// Delete self if turns passed
void TemporaryAi::update(Actor* owner) {
	nbTurns--;
	if (nbTurns == 0) {
		owner->ai = oldAi;
		engine.gui->message(
			tcod::stringf(
				"%s recover%s from the effects.",
				owner == engine.player ? "You" : owner->name,
				owner == engine.player ? "" : "s"));
		delete this;
	}
}

// Replace old ai
void TemporaryAi::applyTo(Actor* actor) {
	oldAi = actor->ai;
	actor->ai = this;
}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns) : TemporaryAi(nbTurns) {}

void ConfusedMonsterAi::update(Actor* owner) {
	Random& rng = Random::instance();
	int dx = rng.getInt(-1, 1);
	int dy = rng.getInt(-1, 1);
	if (dx != 0 || dy != 0) {
		int destx = owner->x + dx;
		int desty = owner->y + dy;
		if (engine.map->canWalk(destx, desty)) {
			owner->x = destx;
			owner->y = desty;
		} else {
			Actor* actor = engine.getActor(destx, desty);
			if (actor) {
				owner->attacker->attack(owner, actor);
			}
		}
	}
	TemporaryAi::update(owner);
}

ConfusedPlayerAi::ConfusedPlayerAi(int nbTurns) : TemporaryAi(nbTurns) {}

void ConfusedPlayerAi::update(Actor* owner) {
	if (owner->destructible && owner->destructible->isDead()) {
		TemporaryAi::update(owner);
		return;
	}
	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Turn not spent, keep IDLEing for effective inputs
		engine.gameStatus = Engine::IDLE;
		return;
	}
	int dx, dy;
	bool isActionPickUp, isActionInventory, isActionControlsMenu, isActionDescend, isActionRest;
	PlayerAi::parseInput(
		dx, dy, isActionPickUp, isActionInventory, isActionControlsMenu, isActionDescend, isActionRest);
	// If turn is spent update engine gamestatus to OTHER_ACTORS_TURN, else return to IDLE
	bool isTurnSpent = false;
	if (dx != 0 || dy != 0) {
		dx = Random::instance().getInt(-1, 1), dy = Random::instance().getInt(-1, 1);
		int tries = 5;
		do {
			tries--;
			if ((dx != 0 || dy != 0) && PlayerAi::moveOrAttack(owner, owner->x + dx, owner->y + dy)) {
				engine.map->computeFov();
				isTurnSpent = true;
			}
		} while (tries > 0 && !isTurnSpent);
		isTurnSpent = true;
	} else if (isActionPickUp) {
		bool foundItem = false;
		for (auto actor : engine.actors)
			if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
				if (actor->pickable->pick(actor, owner)) {
					foundItem = true;
					isTurnSpent = true;
					engine.gui->message(tcod::stringf("You picked up %s.", engine.nameTracker->getDisplayName(actor)));
					break;
				} else {
					engine.gui->message(tcod::stringf("Your inventory is full!"));
					break;
				}
			}
		if (!foundItem) {
			engine.gui->message(tcod::stringf("There's nothing to pick up here."));
		}
	} else if (isActionInventory) {
		PlayerAi::openInventory(owner);
		// Game status will be updated after inventory opening is complete
		// So we could return here and not update game state
		// Also note using inventory actions dont pass confusion timer!!
		return;
	} else if (isActionControlsMenu) {
		new ControlsMenu();
		return;
	} else if (isActionDescend) {
		if (std::tie(owner->x, owner->y) == std::tie(engine.stairs->x, engine.stairs->y)) {
			engine.nextLevel();
			// Although turn not spent, pass confusion timer
			engine.gameStatus = Engine::IDLE;
			TemporaryAi::update(owner);
			return;
		} else {
			engine.gui->message(
				"There are no stairs here.\nDescend by pressing > while standing\ndirectly on top of stairs.");
		}
	} else if (isActionRest) {
		isTurnSpent = true;
	}
	if (isTurnSpent) {
		engine.gameStatus = Engine::OTHER_ACTORS_TURN;
		TemporaryAi::update(owner);
	} else
		engine.gameStatus = Engine::IDLE;
}

FireAi::FireAi(Actor* target, int nbTurns) : target(target), nbTurns(nbTurns) {}

void FireAi::update(Actor* owner) {
	if (nbTurns > 0) {
		nbTurns--;
		if (target && target->destructible && !target->destructible->isDead()) owner->attacker->burn(owner, target);
	}
}

NatureAi::NatureAi(int level) : nbTurnsSinceCreation(nbTurnsSinceCreation), level(level) {}

void NatureAi::update(Actor* owner) {
	Actor* fireActor = NULL;
	for (auto existingActor : engine.actors)
		if (FireAi* existingFireAi = dynamic_cast<FireAi*>(existingActor->ai)) {
			if (existingFireAi->target == engine.player) {
				fireActor = existingActor;
				break;
			}
		}
	bool foundPlayerOnFire = false;
	if (fireActor != NULL) {
		if (fireActor->ai) {
			if (FireAi* fireAi = dynamic_cast<FireAi*>(fireActor->ai)) {
				if (fireAi->nbTurns > 0 && fireAi->target == engine.player) {
					foundPlayerOnFire = true;
				}
			}
		}
	}

	nbTurnsSinceCreation++;

	// Natural Regeneration, 1HP every 2 turns
	if (nbTurnsSinceCreation % 2 == 0) {
		if (engine.player->destructible && !engine.player->destructible->isDead() && !foundPlayerOnFire) {
			engine.player->destructible->heal(1.0F);
		}
	}
}
