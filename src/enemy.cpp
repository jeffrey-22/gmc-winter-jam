#include "main.hpp"

static auto constexpr DESATURATED_GREEN = tcod::ColorRGB{63, 127, 63};

Actor* Enemy::newEnemy(int x, int y) {
	Actor* enemy = new Actor(x, y, 'M', "Monster", DESATURATED_GREEN);
	engine.actors.push_back(enemy);
	return enemy;
}

void Enemy::setRandomEnemyByFloor(Actor* enemy) {
	std::vector<int> possibleEnemyIndices;
	switch (engine.level) {
		case 1:
			possibleEnemyIndices = {0};
			break;
		case 2:
		case 3:
			possibleEnemyIndices = {0, 1};
			break;
		case 4:
		case 5:
		case 6:
			possibleEnemyIndices = {0, 1, 2};
			break;
		case 7:
		case 8:
			possibleEnemyIndices = {0, 1, 3};
			break;
		case 9:
		case 10:
			possibleEnemyIndices = {1, 3};
			break;
		case 11:
		case 12:
		case 13:
			possibleEnemyIndices = {1, 3, 4};
			break;
		case 14:
			possibleEnemyIndices = {4, 5};
			break;
		case 15:
		case 16:
			possibleEnemyIndices = {4, 5, 6};
			break;
		case 17:
		case 18:
			possibleEnemyIndices = {6, 7};
			break;
		case 19:
		case 20:
			possibleEnemyIndices = {6, 7, 8};
			break;
	}
	int enemyIndex = possibleEnemyIndices[Random::instance().getInt(0, (int)(possibleEnemyIndices.size()) - 1)];
	switch (enemyIndex) {
		case 0:
			setOrc(enemy);
			break;
		case 1:
			setGoblin(enemy);
			break;
		case 2:
			setGremlin(enemy);
			break;
		case 3:
			setElf(enemy);
			break;
		case 4:
			setOgre(enemy);
			break;
		case 5:
			setLich(enemy);
			break;
		case 6:
			setTroll(enemy);
			break;
		case 7:
			setDragon(enemy);
			break;
		case 8:
			setCentaur(enemy);
			break;
	}
}

void Enemy::setOrc(Actor* enemy) {
	enemy->ch = 'o';
	enemy->name = "orc";
	enemy->destructible = new MonsterDestructible(20, 0, "dead orc");
	enemy->attacker = new Attacker(8);
	enemy->ai = new MonsterAi();
}

void Enemy::setGoblin(Actor* enemy) {
	enemy->ch = 'g';
	enemy->name = "goblin";
	enemy->destructible = new MonsterDestructible(58, 5, "goblin corpse");
	enemy->attacker = new Attacker(13);
	enemy->ai = new MonsterAi();
}

void Enemy::setGremlin(Actor* enemy) {
	enemy->ch = 'G';
	enemy->name = "gremlin";
	enemy->destructible = new MonsterDestructible(69, 10, "gremlin corpse");
	enemy->attacker = new Attacker(15);
	enemy->ai = new GremlinAi();
}

void Enemy::setElf(Actor* enemy) {
	enemy->ch = 'e';
	enemy->name = "elf";
	enemy->destructible = new MonsterDestructible(72, 11, "elf corpse");
	enemy->attacker = new Attacker(29);
	enemy->ai = new ElfAi();
}

void Enemy::setOgre(Actor* enemy) {
	enemy->ch = 'O';
	enemy->name = "ogre";
	enemy->destructible = new MonsterDestructible(72, 12, "ogre corpse");
	enemy->attacker = new Attacker(43);
	enemy->ai = new MonsterAi();
}

void Enemy::setLich(Actor* enemy) {
	enemy->ch = 'L';
	enemy->name = "lich";
	enemy->destructible = new MonsterDestructible(72, 11, "dust");
	enemy->attacker = new Attacker(32);
	enemy->ai = new LichAi();
}

void Enemy::setTroll(Actor* enemy) {
	enemy->ch = 'T';
	enemy->name = "troll";
	enemy->destructible = new MonsterDestructible(83, 15, "troll carcass");
	enemy->attacker = new Attacker(42);
	enemy->ai = new MonsterAi();
}

void Enemy::setDragon(Actor* enemy) {
	enemy->ch = 'D';
	enemy->name = "dragon";
	enemy->destructible = new MonsterDestructible(90, 15, "dragon corpse");
	enemy->attacker = new Attacker(55);
	enemy->ai = new DragonAi();
}

void Enemy::setCentaur(Actor* enemy) {
	enemy->ch = 'C';
	enemy->name = "centaur";
	enemy->destructible = new MonsterDestructible(98, 16, "centaur carcass");
	enemy->attacker = new Attacker(62);
	enemy->ai = new MonsterAi();
}
