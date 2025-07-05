#pragma once

#include "main.hpp"

class Ai {
   public:
	virtual void update(Actor* owner) = 0;

	virtual ~Ai() {};
};

class PlayerAi : public Ai {
   public:
	void update(Actor* owner);

   protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	void openInventory(Actor* owner);
};

class MonsterAi : public Ai {
   public:
	void update(Actor* owner);

   protected:
	int moveCount;
	// How many turns the monster chases the player after losing his sight
	static const int TRACKING_TURNS = 3;

	void moveOrAttack(Actor* owner, int targetx, int targety);
};

class ConfusedMonsterAi : public Ai {
   public:
	ConfusedMonsterAi(int nbTurns, Ai* oldAi);
	void update(Actor* owner);

   protected:
	int nbTurns;
	Ai* oldAi;
};
