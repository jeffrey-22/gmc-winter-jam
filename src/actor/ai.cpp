#include <cassert>

#include "main.hpp"

static constexpr auto RED = tcod::ColorRGB{255, 0, 0};

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
			return;
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
	globalTurn++;
	if (engine.map->isInFov(owner->x, owner->y)) {
		// we can see the player. move towards him
		chasingTurn = CHASING_TURN;
		wanderingTurn = 0;
	} else {
		chasingTurn--;
		chasingTurn = std::max(chasingTurn, 0);
	}
	if (chasingTurn > 0) {
		auto [dx, dy] = engine.map->directionAtTarget(engine.player->x, engine.player->y, owner->x, owner->y);
		moveOrAttack(owner, dx, dy);
	} else {
		wanderingTurn--;
		if (wanderingTurn <= 0 || owner->getDistance(targetX, targetY) <= 3.0F) {
			wanderingTurn = WANDERING_CHANGE_TARGET_TURN;
			int tries = 10;
			do {
				tries--;
				targetX = Random::instance().getInt(0, engine.MAP_WIDTH);
				targetY = Random::instance().getInt(0, engine.MAP_HEIGHT);
			} while ((owner->getDistance(targetX, targetY) <= 35.0F || !engine.map->canWalk(targetX, targetY)) &&
					 tries > 0);
			if ((owner->getDistance(targetX, targetY) <= 35.0F || !engine.map->canWalk(targetX, targetY))) {
				int roomIndex = Random::instance().getInt(0, (int)(engine.map->roomRecords.size()) - 1);
				targetX = Random::instance().getInt(
					engine.map->roomRecords[roomIndex][0], engine.map->roomRecords[roomIndex][2]);
				targetY = Random::instance().getInt(
					engine.map->roomRecords[roomIndex][1], engine.map->roomRecords[roomIndex][3]);
			}
		}
		if (globalTurn > 200) {
			targetX = engine.player->x;
			targetY = engine.player->y;
		}
		auto [dx, dy] = engine.map->directionAtTarget(targetX, targetY, owner->x, owner->y);
		moveOrAttack(owner, dx, dy);
	}
}

void MonsterAi::moveOrAttack(Actor* owner, int dx, int dy) {
	int cx = owner->x + dx;
	int cy = owner->y + dy;
	if (engine.player->x == cx && engine.player->y == cy) {
		if (owner->attacker && engine.player->destructible && !engine.player->destructible->isDead()) {
			owner->attacker->attack(owner, engine.player);
		}
		return;
	}
	if (engine.map->canWalk(cx, cy)) {
		owner->x = cx;
		owner->y = cy;
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

NatureAi::NatureAi(int level) : nbTurnsSinceCreation(0), level(level) {}

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
	// Monster spawning
	if (nbTurnsSinceCreation % engine.monsterSpawnRate == engine.monsterSpawnRate - 1) {
		engine.map->addOneNewMonster();
	}
}

void GremlinAi::update(Actor* owner) {
	if (Random::instance().getBool(0.25) && engine.map->isInFov(owner->x, owner->y) &&
		engine.player->getDistance(owner->x, owner->y) <= 1.6F && engine.player && engine.player->destructible &&
		!engine.player->destructible->isDead()) {
		engine.gui->message("The gremlin grins at you.");
	} else
		MonsterAi::update(owner);
}

void ElfAi::update(Actor* owner) {
	if (Random::instance().getBool(0.5)) {
		Actor* healTarget = NULL;
		for (auto actor : engine.actors)
			if (actor != engine.player && actor != owner && actor->getDistance(owner->x, owner->y) <= 5.0F) {
				healTarget = actor;
				break;
			}
		if (healTarget != NULL) {
			HealthEffect effect(30.0F, 0.0F, "The elf casts a healing spell!\n%s recovers %g HP.");
			effect.applyTo(healTarget);
		}
	} else
		MonsterAi::update(owner);
}

void LichAi::update(Actor* owner) {
	if (Random::instance().getBool(0.5) && engine.map->isInFov(owner->x, owner->y) &&
		engine.player->getDistance(owner->x, owner->y) <= 2.9F && engine.player && engine.player->destructible &&
		!engine.player->destructible->isDead()) {
		ConfusionEffect effect;
		engine.gui->message("The Lich casts confusion magic at you!", RED);
		effect.applyTo(engine.player);
	} else
		MonsterAi::update(owner);
}

void DragonAi::update(Actor* owner) {
	if (Random::instance().getBool(0.05)) {
		if (engine.player && engine.player->destructible && !engine.player->destructible->isDead())
			engine.gui->message("The Dragon blows fire at you!", RED);
		owner->attacker->burn(owner, engine.player, 30.0F);
	} else
		MonsterAi::update(owner);
}
