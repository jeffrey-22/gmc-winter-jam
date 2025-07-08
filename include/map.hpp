#pragma once

#include "main.hpp"

struct Tile {
	bool explored;	// has the player already seen this tile ?
	Tile() : explored(false) {}
};

class Map {
   public:
	int width, height;

	Map(int width, int height);
	~Map();
	bool isWalkable(int x, int y) const;
	bool canWalk(int x, int y) const;
	void setWalkable(int x, int y, bool newWalkableValue = true);
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	std::array<int, 2> findSpotsNear(int x, int y);
	void computeFov();
	void render(tcod::Console& console) const;

	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2);
	void addMonsters();
	void addOneNewMonster();
	void addItems();

	// calculate dx, dy for target at (x, y) from (cx, cy). dx, dy may be both 0.
	std::array<int, 2> directionAtTarget(int x, int y, int cx, int cy);

	bool isMapRevealed;
	void revealMap();
	void cancelRevealMap();

	// x1, y1, x2, y2
	std::vector<std::array<int, 4>> roomRecords;

   protected:
	// Tile at (x, y) is indexed at x + y * width
	Tile* tiles;
	TCODMap* map;
	friend class BspListener;
};
