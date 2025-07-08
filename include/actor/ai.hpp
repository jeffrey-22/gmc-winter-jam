#pragma once

#include "main.hpp"

class Ai {
   public:
	virtual void update(Actor* owner) = 0;

	virtual ~Ai() {};
};

class PlayerAi : public Ai {
   public:
	void update(Actor* owner) override;
	static bool moveOrAttack(Actor* owner, int targetx, int targety);
	static void openInventory(Actor* owner);
	static void parseInput(
		int& dx,
		int& dy,
		bool& isActionPickUp,
		bool& isActionInventory,
		bool& isActionControlsMenu,
		bool& isActionDescend,
		bool& isActionRest);
};

class MonsterAi : public Ai {
   public:
	virtual void update(Actor* owner) override;
	virtual void moveOrAttack(Actor* owner, int dx, int dy);

	int wanderingTurn = 0, chasingTurn = 0, targetX = 0, targetY = 0, globalTurn = 0;
	static const int CHASING_TURN = 3;
	static const int WANDERING_CHANGE_TARGET_TURN = 25;
};

class TemporaryAi : public Ai {
   public:
	TemporaryAi(int nbTurns);
	virtual void update(Actor* owner) override;
	void applyTo(Actor* actor);

   protected:
	int nbTurns;
	Ai* oldAi;
};

class ConfusedMonsterAi : public TemporaryAi {
   public:
	ConfusedMonsterAi(int nbTurns);
	void update(Actor* owner) override;
};

class ConfusedPlayerAi : public TemporaryAi {
   public:
	ConfusedPlayerAi(int nbTurns);
	void update(Actor* owner) override;
};

class FireAi : public Ai {
   public:
	FireAi(Actor* target, int nbTurns);
	Actor* target;
	int nbTurns;
	void update(Actor* owner) override;
};

class NatureAi : public Ai {
   public:
	NatureAi(int level);
	int nbTurnsSinceCreation, level;
	void update(Actor* owner) override;
};

class GremlinAi : public MonsterAi {
	void update(Actor* owner) override;
};

class ElfAi : public MonsterAi {
	void update(Actor* owner) override;
};

class LichAi : public MonsterAi {
	void update(Actor* owner) override;
};

class DragonAi : public MonsterAi {
	void update(Actor* owner) override;
};
