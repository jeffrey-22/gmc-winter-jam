#pragma once

#include "main.hpp"

class Container {
   public:
	int size;  // maximum number of actors. 0=unlimited
	std::vector<Actor*> inventory;

	Container(int size);
	~Container();
	bool use(Actor* actor, Actor* wearer);
	bool contains(Actor* actor);
	bool add(Actor* actor);
	bool isIndexValid(int index);
	void remove(Actor* actor);
};
