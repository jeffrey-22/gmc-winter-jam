#pragma once

#include "main.hpp"

class Destructible {
   public:
	float maxHp;  // maximum health points
	float hp;  // current health points
	float defense;	// hit points deflected
	const char* corpseName;	 // the actor's name once dead/destroyed

	Destructible(float maxHp, float defense, const char* corpseName);
	virtual ~Destructible() {};

	inline bool isDead() { return hp <= 0; }
	float takeDamage(Actor* owner, float damage);
	float takeTrueDamage(Actor* owner, float trueDamage);
	virtual void die(Actor* owner);
	float heal(float amount);
	float changeStats(float deltaMaxHp, float deltaDefense);
};

class MonsterDestructible : public Destructible {
   public:
	MonsterDestructible(float maxHp, float defense, const char* corpseName);
	void die(Actor* owner);
};

class PlayerDestructible : public Destructible {
   public:
	PlayerDestructible(float maxHp, float defense, const char* corpseName);
	void die(Actor* owner);
};
