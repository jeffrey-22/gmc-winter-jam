#include <cassert>
#include <queue>

#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 4;

static constexpr int MAX_ID_SCROLL_BY_FLOOR[20] = {4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static constexpr int MIN_TOTAL_ITEMS_BY_FLOOR[20] = {14, 12, 10, 7, 7, 5, 5, 5, 5, 4, 4, 4, 7, 4, 4, 7, 4, 4, 4, 4};
static constexpr int MAX_TOTAL_ITEMS_BY_FLOOR[20] = {14, 12, 10, 9, 8, 7, 6, 6, 6, 6, 6, 6, 9, 6, 6, 9, 6, 6, 6, 6};
static constexpr int ENEMY_DENSITY_BY_FLOOR[20] = {1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4};

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
Map::Map(int width, int height) : width(width), height(height), isMapRevealed(false) {
	roomRecords.clear();
	tiles = new Tile[width * height];
	map = new TCODMap(width, height);

	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, NULL);

	addItems();
	addMonsters();
}

// Destructor
Map::~Map() {
	delete[] tiles;
	delete map;
}

// Is tile walkable
bool Map::isWalkable(int x, int y) const { return map->isWalkable(x, y); }

// Is both tile walkable and no blocking actors are present
bool Map::canWalk(int x, int y) const {
	if (!isWalkable(x, y)) return false;
	for (auto actor : engine.actors)
		if (actor->x == x && actor->y == y && actor->blocks) return false;
	return true;
}

// Set tile to be walkable
void Map::setWalkable(int x, int y, bool newWalkableValue) {
	map->setProperties(x, y, map->isTransparent(x, y), newWalkableValue);
}

// Has the tile been explored by the player before
bool Map::isExplored(int x, int y) const { return tiles[x + y * width].explored; }

// Find spots near (x, y) that are empty and not blocked. If none find, {-1, -1} is returned
std::array<int, 2> Map::findSpotsNear(int x, int y) {
	std::vector<std::pair<std::pair<int, int>, std::array<int, 2>>> candidates = {};
	Random& rng = Random::instance();
	for (int dx = -3; dx <= 3; dx++)
		for (int dy = -3; dy <= 3; dy++) {
			int cx = x + dx, cy = y + dy;
			bool hasBlocker = false;
			for (auto actor : engine.actors)
				if (actor->x == cx && actor->y == cy && actor->blocks) {
					hasBlocker = true;
					break;
				}
			if (map->isWalkable(cx, cy) && !hasBlocker) {
				candidates.push_back({{(cx - x) * (cx - x) + (cy - y) * (cy - y), rng.getInt(0, 10000)}, {cx, cy}});
			}
		}
	if (candidates.empty()) return {-1, -1};
	std::sort(candidates.begin(), candidates.end());
	return candidates[0].second;
}

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
				} else if (isExplored(x, y) || isMapRevealed) {
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
	roomRecords.push_back({x1, y1, x2, y2});
	if (first) {
		// Put the player in the first room
		engine.player->x = (x1 + x2) / 2;
		engine.player->y = (y1 + y2) / 2;
	}
	Random& rng = Random::instance();
	// Set stairs position as last room created
	engine.stairs->x = (x1 + x2) / 2;
	engine.stairs->y = (y1 + y2) / 2;
}

void Map::addMonsters() {
	Random& rng = Random::instance();
	for (auto [x1, y1, x2, y2] : roomRecords) {
		for (int x = x1; x <= x2; x++)
			for (int y = y1; y <= y2; y++)
				if (rng.getBool(1.0F * ENEMY_DENSITY_BY_FLOOR[engine.level - 1] / 100) &&
					engine.getActor(x, y) == NULL) {
					Actor* enemy = Enemy::newEnemy(x, y);
					Enemy::setRandomEnemyByFloor(enemy);
				}
	}
}

#include <iostream>
// Add a monster
void Map::addOneNewMonster() {
	Random& rng = Random::instance();
	int tries = 10;
	int x, y;
	do {
		tries--;
		int roomIndex = rng.getInt(0, (int)(roomRecords.size()) - 1);
		auto [x1, y1, x2, y2] = roomRecords[roomIndex];
		x = rng.getInt(x1, x2);
		y = rng.getInt(y1, y2);
	} while ((engine.getActor(x, y) != NULL || engine.player->getDistance(x, y) <= 12.0F) && (tries > 0));

	if (engine.getActor(x, y) == NULL && engine.player->getDistance(x, y) > 12.0F) {
		Actor* enemy = Enemy::newEnemy(x, y);
		Enemy::setRandomEnemyByFloor(enemy);
	}
}

void Map::addItems() {
	Random& rng = Random::instance();
	int nbItems = rng.getInt(MIN_TOTAL_ITEMS_BY_FLOOR[engine.level - 1], MAX_TOTAL_ITEMS_BY_FLOOR[engine.level - 1]);
	int nbIDScrolls = rng.getInt(0, MAX_ID_SCROLL_BY_FLOOR[engine.level - 1]);
	for (int i = 0; i < nbItems + nbIDScrolls; i++) {
		int roomIndex = rng.getInt(0, (int)(roomRecords.size()) - 1);
		int x, y;
		auto [x1, y1, x2, y2] = roomRecords[roomIndex];
		x = rng.getInt(x1, x2);
		y = rng.getInt(y1, y2);
		if (engine.getActor(x, y) == NULL) {
			Actor* item = Item::newItem(x, y);
			if (i < nbIDScrolls)
				Item::setScrollOfIdentify(item);
			else
				Item::setRandomItem(item);
		}
	}
}

std::array<int, 2> Map::directionAtTarget(int x, int y, int cx, int cy) {
	const int INF = 10000;
	std::vector<std::vector<int>> dist(width, std::vector<int>(height, INF));

	std::priority_queue<
		std::pair<int, std::pair<int, int>>,
		std::vector<std::pair<int, std::pair<int, int>>>,
		std::greater<>>
		pq;

	dist[x][y] = 0;
	pq.push({0, {x, y}});

	const int dx[9] = {-1, -1, -1, 0, 0, 1, 1, 1, 0};
	const int dy[9] = {-1, 0, 1, -1, 1, -1, 0, 1, 0};

	while (!pq.empty()) {
		auto [d, pos] = pq.top();
		pq.pop();
		int x = pos.first;
		int y = pos.second;

		if (d > dist[x][y]) continue;  // Already found a better path

		for (int dir = 0; dir < 8; dir++) {
			int nx = x + dx[dir];
			int ny = y + dy[dir];

			if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
			if ((nx != cx || ny != cy) && !canWalk(nx, ny)) continue;

			int nd = d + 10;  // Cost to move is 10
			if (dx[dir] != 0 && dy[dir] != 0) nd = d + 11;
			if (dist[nx][ny] == INF || nd < dist[nx][ny]) {
				dist[nx][ny] = nd;
				pq.push({nd, {nx, ny}});
			}
		}
	}
	int answerdx = 0, answerdy = 0;
	int bestDist = INF;
	for (int dir = 0; dir < 9; dir++) {
		int nx = cx + dx[dir], ny = cy + dy[dir];
		if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
		if ((nx != cx || ny != cy) && !canWalk(nx, ny) && dist[nx][ny] > 0) continue;
		if (dist[nx][ny] < bestDist) {
			bestDist = dist[nx][ny];
			answerdx = dx[dir];
			answerdy = dy[dir];
		}
	}
	return {answerdx, answerdy};
}

void Map::revealMap() { isMapRevealed = true; }

void Map::cancelRevealMap() { isMapRevealed = false; }
