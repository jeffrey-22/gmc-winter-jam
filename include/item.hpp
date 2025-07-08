#pragma once

#include "main.hpp"

class Item {
   public:
	static Actor* newItem(int x, int y);

	static void setRandomItem(Actor* item);
	static void setRandomPotion(Actor* item);
	static void setRandomScroll(Actor* item);

	static void setPotionOfFullHealing(Actor* item);
	static void setPotionOfStrength(Actor* item);
	static void setPotionOfProtection(Actor* item);
	static void setPotionOfPoison(Actor* item);
	static void setPotionOfAmnesia(Actor* item);
	static void setPotionOfConfusion(Actor* item);
	static void setPotionOfFire(Actor* item);

	static void setScrollOfIdentify(Actor* item);
	static void setScrollOfTeleportation(Actor* item);
	static void setScrollOfMapping(Actor* item);
	static void setScrollOfConfusion(Actor* item);
	static void setScrollOfFireball(Actor* item);
	static void setScrollOfLiquify(Actor* item);
	static void setScrollOfSummonMonsters(Actor* item);
};
