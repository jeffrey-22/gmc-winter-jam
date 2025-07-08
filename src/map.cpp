#include <cassert>

#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;

static const int MAX_ROOM_MONSTERS = 3;

static const int MAX_ROOM_ITEMS = 100;	// TODO: adjust

static constexpr int MAX_ID_SCROLL_BY_FLOOR[20] = {4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static constexpr int MIN_TOTAL_ITEMS_BY_FLOOR[20] = {13, 9, 8, 5, 5, 5, 4, 4, 4, 4, 4, 4, 7, 4, 4, 7, 4, 4, 4, 4};
static constexpr int MAX_TOTAL_ITEMS_BY_FLOOR[20] = {13, 9, 8, 7, 7, 6, 6, 6, 6, 6, 6, 6, 9, 6, 6, 9, 6, 6, 6, 6};
static constexpr int ENEMY_DENSITY_BY_FLOOR[20] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3};

static auto constexpr LIGHT_YELLOW = tcod::ColorRGB{255, 255, 63};
static auto constexpr VIOLET = tcod::ColorRGB{127, 0, 255};

// Bsp Listener class for randomly creating rooms in the Bsp tree algorithm
class BspListener : public ITCODBspCallback {
   private:
	Map& map;  // a map to dig
	int roomNum;  // room number
	int lastx, lasty;  // center of the last room
   public:
	BspListener(Map& map) : map(map), roomNum(0) {}
	bool visitNode(TCODBsp* node, void* userData) {
		if (node->isLeaf()) {
			int x, y, w, h;
			// dig a room
			Random& rng = Random::instance();
			w = rng.getInt(ROOM_MIN_SIZE, node->w - 2);
			h = rng.getInt(ROOM_MIN_SIZE, node->h - 2);
			x = rng.getInt(node->x + 1, node->x + node->w - w - 1);
			y = rng.getInt(node->y + 1, node->y + node->h - h - 1);
			map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);
			if (roomNum != 0) {
				// dig a corridor from last room
				map.dig(lastx, lasty, x + w / 2, lasty);
				map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
			}
			lastx = x + w / 2;
			lasty = y + h / 2;
			roomNum++;
		}
		return true;
	}
};

// Create map with (width, height), so x < width and y < height. Players are already created
Map::Map(int width, int height) : width(width), height(height) {
	tiles = new Tile[width * height];
	map = new TCODMap(width, height);
	// TODO: Create rooms randomly
	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, NULL);
}

// Destructor
Map::~Map() {
	delete[] tiles;
	delete map;
}

// Is tile walkable
bool Map::isWalkable(int x, int y) const { return map->isWalkable(x, y); }

// Is both tile walkable and no actors are present
bool Map::canWalk(int x, int y) const {
	if (!isWalkable(x, y)) return false;
	for (auto actor : engine.actors)
		if (actor->x == x && actor->y == y) return false;
	return true;
}

// Set tile to be walkable
void Map::setWalkable(int x, int y, bool newWalkableValue) {
	map->setProperties(x, y, map->isTransparent(x, y), newWalkableValue);
}

// Has the tile been explored by the player before
bool Map::isExplored(int x, int y) const { return tiles[x + y * width].explored; }

// Is the tile in FoV, but computeFov() must be called first
bool Map::isInFov(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	if (map->isInFov(x, y)) {
		tiles[x + y * width].explored = true;
		return true;
	}
	return false;
}

// Compute new FoV based on fovRadius set in Engine, quite expensive
void Map::computeFov() { map->computeFov(engine.player->x, engine.player->y, engine.fovRadius); }

// Draw map background tiles on the console
void Map::render(tcod::Console& console) const {
	static const TCOD_color_t darkWall = {0, 0, 100};
	static const TCOD_color_t darkGround = {70, 40, 30};
	static const TCOD_color_t lightWall = {130, 110, 150};
	static const TCOD_color_t lightGround = {200, 130, 50};
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (console.in_bounds({x, y})) {
				if (isInFov(x, y)) {
					console[{x, y}].bg = isWalkable(x, y) ? lightGround : lightWall;
				} else if (isExplored(x, y)) {
					console[{x, y}].bg = isWalkable(x, y) ? darkGround : darkWall;
				}
			}
		}
	}
}

// Dig out a rectangle
void Map::dig(int x1, int y1, int x2, int y2) {
	if (x2 < x1) std::swap(x1, x2);
	if (y2 < y1) std::swap(y1, y2);
	assert(x1 >= 0 && x2 < width);	// x must be in bounds
	assert(y1 >= 0 && y2 < width);	// y must be in bounds
	for (int tilex = x1; tilex <= x2; tilex++) {
		for (int tiley = y1; tiley <= y2; tiley++) {
			map->setProperties(tilex, tiley, true, true);
		}
	}
}

// Create a rectangular room, and if first room the player position is set to be there
void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
	dig(x1, y1, x2, y2);
	if (first) {
		// Put the player in the first room
		engine.player->x = (x1 + x2) / 2;
		engine.player->y = (y1 + y2) / 2;
	}
	Random rng = Random::instance();
	// Add monsters
	int nbMonsters = rng.getInt(0, MAX_ROOM_MONSTERS);
	while (nbMonsters > 0) {
		int x = rng.getInt(x1, x2);
		int y = rng.getInt(y1, y2);
		if (canWalk(x, y)) {
			addMonster(x, y);
		}
		nbMonsters--;
	}
	// Add items
	int nbItems = rng.getInt(0, MAX_ROOM_ITEMS);
	while (nbItems > 0) {
		int x = rng.getInt(x1, x2);
		int y = rng.getInt(y1, y2);
		if (canWalk(x, y)) {
			addItem(x, y);
		}
		nbItems--;
	}
	// Set stairs position as last room created
	engine.stairs->x = (x1 + x2) / 2;
	engine.stairs->y = (y1 + y2) / 2;
}

// Add a monster
void Map::addMonster(int x, int y) {
	Random rng = Random::instance();
	if (rng.getInt(0, 100) < 80) {
		// create an orc
		Actor* orc = new Actor(x, y, 'o', "orc", tcod::ColorRGB{63, 127, 63});
		orc->destructible = new MonsterDestructible(10, 0, "dead orc");
		orc->attacker = new Attacker(3);
		orc->ai = new MonsterAi();
		engine.actors.push_back(orc);
	} else {
		// create a troll
		Actor* troll = new Actor(x, y, 'T', "troll", tcod::ColorRGB{0, 127, 0});
		troll->destructible = new MonsterDestructible(16, 1, "troll carcass");
		troll->attacker = new Attacker(4);
		troll->ai = new MonsterAi();
		engine.actors.push_back(troll);
	}
}

void Map::addItem(int x, int y) {
	Random rng = Random::instance();
	int dice = rng.getInt(0, 100);

	if (dice < 50) {
		Item::addPotionOfFire(x, y);
	} else {
		Item::addPotionOfProtection(x, y);
	}
	/*
	else if (dice < 20 + 20) {
		// create a scroll of lightning bolt
		Actor* scrollOfLightningBolt = new Actor(x, y, '?', "scroll of lightning bolt", LIGHT_YELLOW);
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->fovOnly = false;
		scrollOfLightningBolt->pickable = new Pickable(
			new TargetSelector(TargetSelector::CLOSEST_MONSTER, 5),
			new HealthEffect(
				-20,
				"A lighting bolt strikes the %s with a loud thunder!\n"
				"The damage is %g hit points."));
		engine.actors.push_back(scrollOfLightningBolt);
		engine.sendToBack(scrollOfLightningBolt);
	} else if (dice < 20 + 20 + 20) {
		// create a scroll of fireball
		Actor* scrollOfFireball = new Actor(x, y, '?', "scroll of fireball", LIGHT_YELLOW);
		scrollOfFireball->blocks = false;
		scrollOfFireball->fovOnly = false;
		scrollOfFireball->pickable = new Pickable(
			new TargetSelector(TargetSelector::SELECTED_RANGE, 3),
			new HealthEffect(-12, "The %s gets burned for %g hit points."));
		engine.actors.push_back(scrollOfFireball);
		engine.sendToBack(scrollOfFireball);
	} else {
		// create a scroll of confusion
		Actor* scrollOfConfusion = new Actor(x, y, '?', "scroll of confusion", LIGHT_YELLOW);
		scrollOfConfusion->blocks = false;
		scrollOfConfusion->fovOnly = false;
		scrollOfConfusion->pickable = new Pickable(
			new TargetSelector(TargetSelector::SELECTED_MONSTER, 5),
			new AiChangeEffect(
				new ConfusedMonsterAi(10), "The eyes of the %s look vacant,\nas he starts to stumble around!"));
		engine.actors.push_back(scrollOfConfusion);
		engine.sendToBack(scrollOfConfusion);
	}
	*/
}
