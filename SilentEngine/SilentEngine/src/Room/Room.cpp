#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), 
	m_enemyShader(nullptr), m_mapShader(nullptr), m_Projectile(nullptr),
	isEnemy(false), isProjectile(false), isClear(false)
{
}

Room::~Room()
{
	//셰이더의 delete는 scene에서 한번에 처리
	//여기서 안함
}

void Room::SetStartPoint(Point * point)
{
	for (UINT i = 0; i < 4; ++i) {
		//m_gatePoint[i] = point[i];
		m_startPoint[i] = point[i];
	}
	

	m_doorRect[0].left			= point[0].xPos-30.0f;
	m_doorRect[0].top			= point[0].zPos + 30.0f;
	m_doorRect[0].right		= point[0].xPos;
	m_doorRect[0].bottom	= point[0].zPos - 30.0f;
	m_startPoint[0].xPos -= 50.0f;
	m_gatePoint[0] = point[1];

	m_doorRect[1].left			= point[1].xPos;
	m_doorRect[1].top			= point[1].zPos + 30.0f;
	m_doorRect[1].right		= point[1].xPos+30.0f;
	m_doorRect[1].bottom	= point[1].zPos - 30.0f;
	m_startPoint[1].xPos += 50.0f;
	m_gatePoint[1] = point[0];

	m_doorRect[2].left			= point[2].xPos - 30.0f;
	m_doorRect[2].top			= point[2].zPos +30.0f;
	m_doorRect[2].right		= point[2].xPos + 30.0f;
	m_doorRect[2].bottom	= point[2].zPos;
	m_startPoint[2].zPos += 50.0f;
	m_gatePoint[2] = point[3];

	m_doorRect[3].left			= point[3].xPos - 30.0f;
	m_doorRect[3].top			= point[3].zPos;
	m_doorRect[3].right		= point[3].xPos + 30.0f;
	m_doorRect[3].bottom	= point[3].zPos-30.0f;
	m_startPoint[3].zPos -= 50.0f;
	m_gatePoint[3] = point[2];
}

void Room::SetNextRoom(UINT * room)
{
	memcpy(m_nextRoom, room, sizeof(UINT) * 4);
}

Point* Room::RegistShader(BasePhysX * phys, bool state, const char& loc)
{
	if (state) {
		if(m_enemyShader)
			m_enemyShader->setPhys(phys);
		if(m_mapShader)
			m_mapShader->SetPhys(phys);

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
	if (isEnemy)
		m_enemyShader->Render(pd3dCommandList, pCamera);
	if (isProjectile)
		m_Projectile->Render(pd3dCommandList, pCamera);
}

void Room::Animate(float fTime, XMFLOAT3& playerPos, Door& change)
{
	//맵은 애니메이션이 없기에 애니메이트 X
	//m_mapShader->Animate(fTime);

	//방 이동함수
	if (isClear) { //현재 방이 클리어상태인 경우에만 다음 방으로 이동할 수 있음
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
	if (isEnemy)
		m_enemyShader->Animate(fTime);
	if (isProjectile)
		m_Projectile->Animate(fTime);
}