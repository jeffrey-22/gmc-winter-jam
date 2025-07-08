#include <cstdio>

#include "main.hpp"

Actor::Actor(int x, int y, char ch, const char* name, const TCOD_color_t& color)
	: x(x),
	  y(y),
	  ch(ch),
	  name(name),
	  color(color),
	  blocks(true),
	  fovOnly(true),
	  attacker(NULL),
	  destructible(NULL),
	  ai(NULL),
	  pickable(NULL),
	  container(NULL) {}

Actor::~Actor() {
	if (attacker) delete attacker;
	if (destructible) delete destructible;
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete container;
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrtf(1.0f * (dx * dx + dy * dy));
}

// Draw actor tiles on the console according to the members: character ch and color col
void Actor::render(tcod::Console& console) const {
	if (console.in_bounds({x, y})) {
		console.at({x, y}).ch = ch;
		console[{x, y}].fg = color;
	}
}

// If has AI, have the component update
void Actor::update() {
	if (ai != NULL) ai->update(this);
}
