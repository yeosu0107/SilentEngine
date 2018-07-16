#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), 
	m_enemyShader(nullptr), m_mapShader(nullptr), m_Projectile(nullptr),
	isEnemy(false), isProjectile(false), isClear(false), isStatBouns(true)
{
}

Room::~Room()
{
	//셰이더의 delete는 scene에서 한번에 처리
	//여기서 안함
}

void Room::SetStartPoint(XMFLOAT3 * point)
{
	SetSpawnPoint(point);
	for (UINT i = 0; i < 4; ++i) {
		m_startPoint[i] = point[i];
	}
	m_yPos = point->y;
	m_doorRect[0].left			= point[0].x-30.0f;
	m_doorRect[0].top			= point[0].z + 30.0f;
	m_doorRect[0].right		= point[0].x;
	m_doorRect[0].bottom	= point[0].z - 30.0f;
	m_startPoint[0].x -= 50.0f;
	m_gatePoint[0] = point[1];

	m_doorRect[1].left			= point[1].x;
	m_doorRect[1].top			= point[1].z + 30.0f;
	m_doorRect[1].right		= point[1].x+30.0f;
	m_doorRect[1].bottom	= point[1].z - 30.0f;
	m_startPoint[1].x += 50.0f;
	m_gatePoint[1] = point[0];

	m_doorRect[2].left			= point[2].x - 30.0f;
	m_doorRect[2].top			= point[2].z +30.0f;
	m_doorRect[2].right		= point[2].x + 30.0f;
	m_doorRect[2].bottom	= point[2].z;
	m_startPoint[2].z += 50.0f;
	m_gatePoint[2] = point[3];

	m_doorRect[3].left			= point[3].x - 30.0f;
	m_doorRect[3].top			= point[3].z;
	m_doorRect[3].right		= point[3].x + 30.0f;
	m_doorRect[3].bottom	= point[3].z-30.0f;
	m_startPoint[3].z -= 50.0f;
	m_gatePoint[3] = point[2];
}

void Room::SetNextRoom(UINT * room)
{
	memcpy(m_nextRoom, room, sizeof(UINT) * 4);
}

void Room::SetSpawnPoint(XMFLOAT3 * point)
{
	float x	= point[0].x;
	float y	= point[0].y;
	float z	= point[3].z;

	m_spawnPoint[0].x = x / 2;		m_spawnPoint[0].z = - z / 2;
	m_spawnPoint[1].x = x / 2;		m_spawnPoint[1].z = z / 2;
	m_spawnPoint[2].x = - x / 2;   m_spawnPoint[2].z = -z / 2;
	m_spawnPoint[3].x = -x / 2;    m_spawnPoint[3].z = z / 2;
	m_spawnPoint[4].x = -x / 2;    m_spawnPoint[4].z = 0;
	m_spawnPoint[5].x = x / 2;		m_spawnPoint[5].z = 0;
	
	for (int i = 0; i < 6; ++i)
		m_spawnPoint[i].y = y;

	m_yPos = y;
}

void Room::SetFirePosition(XMFLOAT3 * pos)
{
	m_pFirePos = pos;
}

void Room::ResetFire()
{
	m_pFires->SetLive(false, 10);
	m_pFires->SetPos(m_pFirePos, 10);
}

XMFLOAT3* Room::RegistShader(BasePhysX * phys, bool state, const char& loc)
{
	if (state) {
		if (m_enemyShader && !isClear) {
			m_enemyShader->setPhys(phys);
			m_enemyShader->setPosition(m_spawnPoint);
		}
		if(m_mapShader)
			m_mapShader->setPhys(phys);

		return  &m_startPoint[loc];
	}
	else {
		if (m_enemyShader)
			m_enemyShader->releasePhys();
		if (m_mapShader)
			m_mapShader->releasePhys();
		if (m_Projectile)
			m_Projectile->releasePhys();

		return nullptr;
	}
}

void Room::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	m_mapShader->Render(pd3dCommandList, pCamera);
	if(m_pFires)
		m_pFires->Render(pd3dCommandList, pCamera);
	if (!isClear) {
		if (isEnemy) {
			m_enemyShader->Render(pd3dCommandList, pCamera);
		}
		if (isProjectile)
			m_Projectile->Render(pd3dCommandList, pCamera);
	}
}

void Room::RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera, XMFLOAT3& cameraPos, float offset)
{
	m_mapShader->RenderToDepthBuffer(pd3dCommandList, pCamera, cameraPos, offset);
	if (!isClear) {
		if (isEnemy) {
			m_enemyShader->RenderToDepthBuffer(pd3dCommandList, pCamera, cameraPos, offset);
		}
	}
}

void Room::Animate(float fTime, XMFLOAT3& playerPos, Door& change)
{
	//맵은 애니메이션이 없기에 애니메이트 X
	//m_mapShader->Animate(fTime);
	//방 이동함수
	if (isClear) { //현재 방이 클리어상태인 경우에만 다음 방으로 이동할 수 있음
		if (m_type == BOSS_ROOM) //보스방이면 다음 방 이동 없음 
		{
#ifdef _DEBUG
			cout << "clear\n";
#endif
			return;
		}
		for (UINT i = 0; i < 4; ++i) {
			if (playerPos.x > m_doorRect[i].left && playerPos.x < m_doorRect[i].right &&
				playerPos.z < m_doorRect[i].top && playerPos.z > m_doorRect[i].bottom) {
				switch (i) { //문 앞에 충돌영역을 두고 충돌체크, 영역과 충돌시 다음 방 이동
				case START_EAST: //다음 방넘버, 시작지점, 방을 바꾸라는 bool값
					if(m_nextRoom[START_WEST]!= BLANK_ROOM)
						change = Door(m_nextRoom[START_WEST], START_WEST, true);
					break;
				case START_WEST:
					if(m_nextRoom[START_EAST]!=BLANK_ROOM)
						change = Door(m_nextRoom[START_EAST], START_EAST, true);
					break;
				case START_SOUTH:
					if (m_nextRoom[START_NORTH] != BLANK_ROOM)
						change = Door(m_nextRoom[START_NORTH], START_NORTH, true);
					break;
				case START_NORTH:
					if (m_nextRoom[START_SOUTH] != BLANK_ROOM)
						change = Door(m_nextRoom[START_SOUTH], START_SOUTH, true);
					break;
				}
			}
		}
	}
	else {
		if (*GlobalVal::getInstance()->getRemainEnemy() <= 0) {
			isClear = true;
			return;
		}
		if (isEnemy)
			m_enemyShader->Animate(fTime);
		if (isProjectile)
			m_Projectile->Animate(fTime);
	}
	if (m_pFires)
		m_pFires->Animate(fTime);
}