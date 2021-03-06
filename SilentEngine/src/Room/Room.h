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

const char BOSS_ROOM = 10;

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
	XMFLOAT3							m_gatePoint[4];
	XMFLOAT3							m_startPoint[4]; //플레이어 시작지점 4방향 문
	RECT									m_doorRect[4];
	UINT									m_nextRoom[4];
	XMFLOAT3							m_spawnPoint[10];
	XMFLOAT3*						m_pFirePos;
	UINT*							m_pFireType;
	float									m_yPos = -180.0f;
	
	UINT									m_type;
	UINT									m_numEnemy;

	bool									isClear;
	bool									isEnemy;
	bool									isProjectile;
	bool									isStatBouns;

	ModelShader*						m_mapShader;
	ModelShader*						m_enemyShader;

	ProjectileShader*				m_Projectile;
	BillboardShader*					m_pFires;
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
	void SetFireShader(BillboardShader* fires) {
		m_pFires = fires;
	}
	void SetEnemyShader(ModelShader* enemy) {
		isEnemy = true;
		m_enemyShader = enemy; 
	}
	void SetProjectileShader(ProjectileShader* projectile) {
		if (projectile == nullptr) 
			isProjectile = false;
		else
			isProjectile = true;
		m_Projectile = projectile;
	}

	void SetStartPoint(XMFLOAT3* point);
	void SetNextRoom(UINT *room);
	void SetSpawnPoint(XMFLOAT3* point);
	void SetSpawnPoint(UINT num, XMFLOAT2* point);
	void SetFirePosition(XMFLOAT3* pos);
	void SetFireType(UINT* type);
	void ResetFire();
	void setType(UINT type) { m_type = type; }

	bool IsEnemy() const { return isEnemy; }
	bool IsProjectile() const { return isProjectile; }
	bool IsClear() const { return isClear; }
	bool IsStatBouns() const { return isStatBouns; }
	
	void SetClear(bool clear);
	void SetStatBouns(bool stat) { isStatBouns = stat; }

	XMFLOAT3* RegistShader(BasePhysX* phys, bool state, const char& loc);
	XMFLOAT3* GetGatePos() { return m_gatePoint; }
	UINT getType() const { return m_type; }
	UINT* getNextRoom() { return m_nextRoom; }
	UINT getNumEnemy() const { return m_numEnemy; }
	ModelShader* GetMapShader() { return m_mapShader; }
	ModelShader* GetEnemyShader() { 
		return m_enemyShader; 
	}
	ProjectileShader* GetProjectileShader() { return m_Projectile; }

	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	
	void RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera, XMFLOAT3& cameraPos, float offset);
	void Animate(float fTime, XMFLOAT3& playerPos, Door& change);
};