#pragma once

#include "main.hpp"

class Item {
   public:
	static void addPotionOfFullHealing(int x, int y);
	static void addPotionOfStrength(int x, int y);
	static void addPotionOfProtection(int x, int y);
	static void addPotionOfPoison(int x, int y);
	static void addPotionOfAmnesia(int x, int y);
	static void addPotionOfConfusion(int x, int y);
	static void addPotionOfFire(int x, int y);

	static void addScrollOfIdentify(int x, int y);
};
