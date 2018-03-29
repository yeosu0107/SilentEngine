#pragma once

#include "..\Model\ModelShader.h"
#include "..\Object\Enemy.h"
#include "..\Shaders\EnemyShader.h"
#include "..\PhysX\BasePhysX.h"

struct Point
{
	int xPos, yPos, zPos;
	Point() : xPos(0), yPos(0), zPos(0) { }
	Point(int x, int y, int z) : xPos(x), yPos(y), zPos(z) { }
	~Point() { }
};

class Room
{
private:
	Point									m_startPoint[4]; //플레이어 시작지점 4방향 문
	UINT									m_type;

	ModelShader*						m_mapShader;
	EnemyShader<Enemy>*		m_enemyShader;
public:
	static enum RoomType {
		ICE=0, NORMAL=1
	};

	Room(UINT type);
	~Room();

	void SetMapShader(ModelShader* map) { 
		m_mapShader = map; 
	}
	void SetEnemyShader(EnemyShader<Enemy>* enemy) { 
		m_enemyShader = enemy; 
	}

	void RegistShader(BasePhysX* phys, bool state);

	ModelShader* GetMapShader() { return m_mapShader; }
	EnemyShader<Enemy>* GetEnemyShader() { return m_enemyShader; }
};