#pragma once

#include "main.hpp"

class Attacker {
   public:
	float power;  // hit points given

	Attacker(float power);
	void attack(Actor* owner, Actor* target);
	void burn(Actor* owner, Actor* target);
	void changePower(Actor* owner, float deltaPower);
};
