#pragma once

#include "..\Model\ModelShader.h"
#include "..\Object\Enemy.h"
#include "..\Shaders\EnemyShader.h"
#include "..\Shaders\ProjectileShader.h"
#include "..\PhysX\BasePhysX.h"

const char START_NON		= 4;
const char START_EAST		= 0;
const char START_WEST		= 1;
const char START_SOUTH	= 2;
const char START_NORTH	= 3;
const UINT START_ROOM = 100;

struct Door
{
	UINT									m_roomNum;
	UINT									m_dir;
	bool									m_isChange;

	Door() : m_roomNum(START_ROOM), m_dir(START_NON),
		m_isChange(false) {}
	Door(UINT num, UINT dir, bool change) :
		m_roomNum(num), m_dir(dir), m_isChange(change) {}
};

class Room
{
private:
	Point									m_startPoint[4]; //플레이어 시작지점 4방향 문
	RECT									m_doorRect[4];
	UINT									m_type;

	bool									isClear;
	bool									isEnemy;
	bool									isProjectile;

	ModelShader*						m_mapShader;
	EnemyShader<Enemy>*		m_enemyShader;
	ProjectileShader*				m_Projectile;
public:
	static enum RoomType {
		brick=0, brick1=1, ice=2, ice2, 
		soil, soil2, stone, stone2, tree, tree2
	};

	Room(UINT type);
	~Room();

	void SetMapShader(ModelShader* map) { 
		m_mapShader = map; 
	}
	void SetEnemyShader(EnemyShader<Enemy>* enemy) { 
		isEnemy = true;
		m_enemyShader = enemy; 
	}
	void SetProjectileShader(ProjectileShader* projectile) {
		isProjectile = true;
		m_Projectile = projectile;
	}

	void SetStartPoint(Point* point);

	bool IsEnemy() const { return isEnemy; }
	bool IsProjectile() const { return isProjectile; }

	Point* RegistShader(BasePhysX* phys, bool state, const char& loc);

	ModelShader* GetMapShader() { return m_mapShader; }
	EnemyShader<Enemy>* GetEnemyShader() { return m_enemyShader; }
	ProjectileShader* GetProjectileShader() { return m_Projectile; }

	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	void Animate(float fTime, XMFLOAT3& playerPos, Door& change);
};