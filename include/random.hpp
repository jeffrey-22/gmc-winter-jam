#pragma once

#include <random>

class Random {
   public:
	Random();
	static Random& instance();
	int operator()();
	int getBoundedInt(int minValue, int maxValue);
	int getInt(int minValue, int maxValue);
	bool getBool(double trueChance = 0.50);
	double getDouble();
	double getBoundedDouble(double minValue, double maxValue);
	void resetSeed(unsigned int newSeed);
	void resetSeed();
	std::mt19937 rng;

   private:
	static unsigned getSystemClock();
};
