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
const char BLANK_ROOM = 100;

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
	Point									m_gatePoint[4];
	Point									m_startPoint[4]; //플레이어 시작지점 4방향 문
	RECT									m_doorRect[4];
	UINT									m_nextRoom[4];
	
	UINT									m_type;

	bool									isClear;
	bool									isEnemy;
	bool									isProjectile;

	ModelShader*						m_mapShader;
	ModelShader*		m_enemyShader;
	ProjectileShader*				m_Projectile;
public:
	UINT									m_mapPosX, m_mapPosY;
	enum RoomType {
		NONE=0, 
		brick=1, brick1=2, ice=3, ice2=4, 
		soil, soil2, stone, stone2, tree, tree2
	};
	Room(UINT type);
	~Room();

	void SetMapShader(ModelShader* map) { 
		m_mapShader = map; 
	}
	void SetEnemyShader(ModelShader* enemy) {
		isEnemy = true;
		m_enemyShader = enemy; 
	}
	void SetProjectileShader(ProjectileShader* projectile) {
		isProjectile = true;
		m_Projectile = projectile;
	}

	void SetStartPoint(Point* point);
	void SetNextRoom(UINT *room);

	bool IsEnemy() const { return isEnemy; }
	bool IsProjectile() const { return isProjectile; }
	bool IsClear() const { return isClear; }

	void SetClear(bool clear) { isClear = clear; }

	Point* RegistShader(BasePhysX* phys, bool state, const char& loc);
	Point* GetGatePos() { return m_gatePoint; }
	UINT getType() const { return m_type; }
	UINT* getNextRoom() { return m_nextRoom; }

	ModelShader* GetMapShader() { return m_mapShader; }
	ModelShader* GetEnemyShader() { return m_enemyShader; }
	ProjectileShader* GetProjectileShader() { return m_Projectile; }

	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	void RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera, XMFLOAT3& cameraPos, float offset);
	void Animate(float fTime, XMFLOAT3& playerPos, Door& change);
};