#pragma once

#include "main.hpp"

class Enemy {
   public:
	static Actor* newEnemy(int x, int y);

	static void setRandomEnemyByFloor(Actor* enemy);

	static void setOrc(Actor* enemy);
	static void setGoblin(Actor* enemy);
	static void setGremlin(Actor* enemy);
	static void setElf(Actor* enemy);
	static void setOgre(Actor* enemy);
	static void setLich(Actor* enemy);
	static void setTroll(Actor* enemy);
	static void setDragon(Actor* enemy);
	static void setCentaur(Actor* enemy);
};
