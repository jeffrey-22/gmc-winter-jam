#include "random.hpp"

#include <cassert>
#include <chrono>

// Singleton instance access
Random& Random::instance() {
	static Random random;
	return random;
}

unsigned Random::getSystemClock() { return std::chrono::system_clock::now().time_since_epoch().count(); }

Random::Random() : rng(std::mt19937(getSystemClock())) {}

// Generate a random signed integer
int Random::operator()() {
	uint_fast32_t result = rng();
	// result is between 0 and 4294967295
	uint_fast32_t constexpr ZERO_VALUE = 2147483648;
	unsigned long long temp = result;
	temp -= ZERO_VALUE;
	return (int)temp;
}

// Generate a non negative bounded integer, both inclusive
int Random::getBoundedInt(int minValue, int maxValue) {
	assert(minValue >= 0 && minValue <= maxValue && maxValue <= 100000000);	 // guarantees randomness
	int delta = maxValue - minValue + 1;

	uint_fast32_t result = rng();
	long long temp = result;

	temp %= delta;
	temp += minValue;

	return (int)temp;
}

int Random::getInt(int minValue, int maxValue) {
	int offset = 0;
	if (minValue < 0) {
		offset = -minValue;
		assert(offset <= 100000000 && maxValue <= 100000000);
		minValue = 0;
		maxValue += offset;
	}
	return getBoundedInt(minValue, maxValue) - offset;
}

// Generate a non negative bounded double in [0, 1]
double Random::getDouble() {
	uint_fast32_t result = rng();
	double normalized = static_cast<double>(result) / static_cast<double>(rng.max());

	return normalized;
}

// Generate a non negative bounded double, both inclusive
double Random::getBoundedDouble(double minValue, double maxValue) {
	assert(minValue >= 0 && minValue <= maxValue);

	double normalized = getDouble();

	return minValue + (maxValue - minValue) * normalized;
}

// Generate a bool with probability trueChance to be true
bool Random::getBool(double trueChance) {
	assert(trueChance >= 0.0 && trueChance <= 1.0);

	uint_fast32_t result = rng();
	uint64_t maxRange = rng.max();
	maxRange++;
	uint64_t decisionBoundary = trueChance * maxRange;

	return result < decisionBoundary ? true : false;
}

// Reset generator to a given new seed
void Random::resetSeed(unsigned int newSeed) { rng = std::mt19937(newSeed); }

// Reset generator to a seed selected by system clock
void Random::resetSeed() { rng = std::mt19937(getSystemClock()); }
