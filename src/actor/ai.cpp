#include <cassert>

#include "main.hpp"

void PlayerAi::update(Actor* owner) {
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	if (engine.lastEventType != SDL_EVENT_KEY_DOWN) {
		// Turn not spent, keep IDLEing for effective inputs
		engine.gameStatus = Engine::IDLE;
		return;
	}
	int dx = 0, dy = 0;
	bool isActionPickUp = false;
	bool isActionInventory = false;
	bool isActionDropItem = false;
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
		case SDLK_PERIOD:
		case SDLK_CLEAR:
		case SDLK_KP_5:
			break;
		case SDLK_G:
			isActionPickUp = true;
			break;
		case SDLK_I:
			isActionInventory = true;
			break;
		case SDLK_D:
			isActionDropItem = true;
			break;
		default:
			break;
	}
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
					engine.gui->message(tcod::stringf("You picked up %s.", actor->name));
					break;
				} else {
					engine.gui->message(tcod::stringf("Your inventory is full!"));
					break;
				}
			}
	} else if (isActionInventory) {
		openInventory(owner);
		// Game status will be updated after inventory opening is complete
		// So we could return here and not update game state
		return;
	} else if (isActionDropItem) {
		new ItemDropMenu(owner);
		return;
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
	// look for corpses
	for (auto actor : engine.actors) {
		if (actor->destructible && actor->destructible->isDead() && actor->x == targetx && actor->y == targety) {
			engine.gui->message(tcod::stringf("There's a %s here\n", actor->name));
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

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns, Ai* oldAi) : nbTurns(nbTurns), oldAi(oldAi) {}

void ConfusedMonsterAi::update(Actor* owner) {
	Random rng = Random::instance();
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
	nbTurns--;
	if (nbTurns == 0) {
		owner->ai = oldAi;
		delete this;
	}
}
