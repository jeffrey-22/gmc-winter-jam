#pragma once

#include "main.hpp"

class Effect {
   public:
	// Returns false if the effect can not be applied
	virtual bool applyTo(Actor* actor) = 0;
};

class HealthEffect : public Effect {
   public:
	float amount, maxAmountOnFull;
	const char* message;

	// message always contains a parameter for the target name (%s) and a parameter for the amount (%g)
	HealthEffect(float amount, float maxAmountOnFull, const char* message);
	bool applyTo(Actor* actor);
};

class AiChangeEffect : public Effect {
   public:
	TemporaryAi* newAi;
	const char* message;

	// message always contains a parameter for the target name (%s)
	AiChangeEffect(TemporaryAi* newAi, const char* message);
	bool applyTo(Actor* actor);
};
