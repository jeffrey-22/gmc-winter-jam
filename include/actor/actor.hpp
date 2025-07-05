#pragma once

#include "main.hpp"

class Actor {
   public:
	int x, y;  // position on map
	char ch;  // ascii code
	TCOD_color_t color;	 // color
	const char* name;  // actor's name in message logs
	bool blocks;  // can we walk on this actor?

	Attacker* attacker;	 // component, deals damage
	Destructible* destructible;	 // component, can be damaged
	Ai* ai;	 // component, self-updating
	Pickable* pickable;	 // component, item that can be picked and used
	Container* container;  // component, item that can contain more actors

	Actor(int x, int y, char ch, const char* name, const TCOD_color_t& color);
	Actor(
		int x,
		int y,
		char ch,
		const char* name,
		const TCOD_color_t& color,
		bool blocks,
		Attacker* attacker,
		Destructible* destructible,
		Ai* ai,
		Pickable* pickable,
		Container* container);
	float getDistance(int cx, int cy) const;
	~Actor();
	void render(tcod::Console& console) const;
	void update();
};
