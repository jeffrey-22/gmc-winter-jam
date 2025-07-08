#pragma once

#include "main.hpp"

class Effect {
   public:
	// Returns false if the effect can not be applied
	virtual bool applyTo(Actor* actor) = 0;
};

class SequentialEffect : public Effect {
   public:
	std::vector<Effect*> memberEffects;

	SequentialEffect(Effect* firstEffect, Effect* secondEffect);
	virtual bool applyTo(Actor* actor) override;
};

class HealthEffect : public Effect {
   public:
	float amount, maxAmountOnFull;
	const char* message;

	// message always contains a parameter for the target name (%s) and a parameter for the amount (%g)
	HealthEffect(float amount, float maxAmountOnFull, const char* message);
	bool applyTo(Actor* actor) override;
};

class AiChangeEffect : public Effect {
   public:
	TemporaryAi* newAi;
	const char* message;

	// message always contains a parameter for the target name (%s)
	AiChangeEffect(TemporaryAi* newAi, const char* message);
	bool applyTo(Actor* actor) override;
};

class PowerChangeEffect : public Effect {
   public:
	float deltaPower;

	PowerChangeEffect(float deltaPower);
	bool applyTo(Actor* actor) override;
};

class AmnesiaEffect : public Effect {
   public:
	AmnesiaEffect();
	bool applyTo(Actor* actor) override;
};

class SetOnFireEffect : public Effect {
   public:
	int nbTurns;
	float damagePerTurn;

	// Use a fire actor per actors on fire.
	// Carefully use the same fire actor and have the ai attack the victim.
	SetOnFireEffect(int nbTurns, float damagePerTurn);
	bool applyTo(Actor* actor) override;
};

class PutOutFireEffect : public Effect {
   public:
	PutOutFireEffect() = default;
	bool applyTo(Actor* actor) override;
};
