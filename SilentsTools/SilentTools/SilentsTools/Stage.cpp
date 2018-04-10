#include "Stage.h"


#include <queue>
#include <time.h>


using namespace std;

namespace STAGE {
	MapGenerator::MapGenerator(int seed, int type) :
		seed(seed), roomType(type)
	{
		
	}

	MapGenerator::~MapGenerator() {

	}

	void MapGenerator::SetMap(int x, int y) {
		default_random_engine dre;
		dre.seed(seed);

		uniform_int_distribution<int> type(1, roomType);
		uniform_int_distribution<int> ui(40, 60);
		uniform_int_distribution<int> index(0, x*y - 1);
		


		Room* shuffledRoom = new Room[x*y];
		int now = 0;
		for (int i = 0; i < y; ++i) {
			for (int j = 0; j < x; ++j) {
				shuffledRoom[now].xPos = j;
				shuffledRoom[now].yPos = i;
				now++;
			}
		}
		currentMap.setMapSize(x, y);


		bool** xroomMap = new bool*[y];
		for (int i = 0; i < y; ++i) {
			*(xroomMap + i) = new bool[x];
		}
		for (int i = 0; i < y; ++i)
			memset(xroomMap[i], false, sizeof(bool)*x);

		float roomPercent = ui(dre)*0.01f;
		int xroomCount = (int)(x * y * roomPercent);
		int curXRoomCount = 0;

		for (int i = 0; i < xroomCount; ++i) {
			Room randomRoom = shuffledRoom[index(dre)];
			if (!xroomMap[randomRoom.yPos][randomRoom.xPos]) {

				xroomMap[randomRoom.yPos][randomRoom.xPos] = true;
				curXRoomCount++;

				if (randomRoom != currentMap.mapCenter() &&
					MapFullyAccessed(xroomMap, curXRoomCount)) {
					continue;
				}
				else {
					xroomMap[randomRoom.yPos][randomRoom.xPos] = false;
					curXRoomCount--;
				}
			}
		}


		for (int i = 0; i < y; ++i) {
			for (int j = 0; j < x; ++j) {
				if (!xroomMap[i][j])
					currentMap.getMapFlags()[i][j] = type(dre);
			}
		}
	}

	bool MapGenerator::MapFullyAccessed(bool** xroomMap, int curObstacleCount) {
		bool** mapFlags = new bool*[currentMap.GetSize().yPos];
		for (int i = 0; i < currentMap.GetSize().yPos; ++i) {
			*(mapFlags + i) = new bool[currentMap.GetSize().xPos];
		}
		for (int i = 0; i < currentMap.GetSize().yPos; ++i) {
			memset(mapFlags[i], false, sizeof(bool)*currentMap.GetSize().xPos);
		}

		queue<Room> xroom;

		xroom.push(currentMap.mapCenter());

		mapFlags[currentMap.mapCenter().yPos][currentMap.mapCenter().xPos] = true;

		int accessibleTileCount = 1;

		while (xroom.size() > 0) {
			Room tmp = xroom.front();
			xroom.pop();

			for (int x = -1; x <= 1; ++x) {
				for (int y = -1; y <= 1; ++y) {
					int tmp_x = tmp.xPos + x;
					int tmp_y = tmp.yPos + y;

					if (x == 0 || y == 0) {
						if (tmp_x >= 0 && tmp_x < currentMap.GetSize().xPos
							&& tmp_y >= 0 && tmp_y < currentMap.GetSize().yPos)
						{
							if (!mapFlags[tmp_y][tmp_x] && !xroomMap[tmp_y][tmp_x])
							{
								mapFlags[tmp_y][tmp_x] = true;
								xroom.push(Room(tmp_x, tmp_y));
								accessibleTileCount++;
							}
						}
					}
				}
			}
		}


		int targetAccessibleTileCount = (currentMap.GetSize().xPos * currentMap.GetSize().yPos - curObstacleCount);
		return targetAccessibleTileCount == accessibleTileCount;
	}

	void MapGenerator::printMap() {
		for (int i = 0; i < currentMap.GetSize().yPos; ++i) {
			for (int j = 0; j < currentMap.GetSize().xPos; ++j) {
				if (currentMap.getMapFlags()[i][j]!=0)
					cout << currentMap.getMapFlags()[i][j]<<"\t";
				else
					cout << 0 << "\t";
			}
			cout << endl;
		}
	}
}