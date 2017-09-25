#pragma once



struct Room
{
	int xPos;
	int yPos;

	Room() {
		xPos = 0;
		yPos = 0;
	}
	Room(int x, int y) {
		xPos = x;
		yPos = y;
	}
	bool operator ==(Room a) {
		return xPos == a.xPos && yPos == a.yPos;
	}
	bool operator !=(Room a) {
		return !(xPos == a.xPos && yPos == a.yPos);
	}
};

class Map
{
private:
	Room mapSize;
	bool** mapFlags;
public:
	Map() {};
	Room mapCenter() {
		return Room(mapSize.xPos / 2, mapSize.yPos / 2);
	}
	Room GetSize() const {
		return mapSize;
	}

	void setMapSize(int x, int y) {
		mapSize.xPos = x;
		mapSize.yPos = y;

		//setFlags
		if (mapFlags)
			delete mapFlags;

		mapFlags = new bool*[mapSize.yPos];
		for (int i = 0; i < mapSize.yPos; ++i) {
			*(mapFlags + i) = new bool[mapSize.xPos];
		}

		for (int i = 0; i < y; ++i) {
			memset(mapFlags[i], false, sizeof(bool)*x);
		}
	}

	bool** getMapFlags() { return mapFlags; }
	int getMapSize() const { return mapSize.xPos * mapSize.yPos; }
};


class MapGenerator
{
private:
	Map currentMap;
	
public:
	MapGenerator();
	~MapGenerator();

	void SetMap(int x, int y);
	bool MapFullyAccessed(bool** obstacleMap, int curObstacleCount);
	void printMap();
	void printMap(bool** tmp);

	Map getCurrentMap() const { return currentMap; }
};

