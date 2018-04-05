#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), 
	m_enemyShader(nullptr), m_mapShader(nullptr), m_Projectile(nullptr),
	isEnemy(false), isProjectile(false), isClear(false)
{
}

Room::~Room()
{
	//���̴��� delete�� scene���� �ѹ��� ó��
	//���⼭ ����
}

void Room::SetStartPoint(Point * point)
{
	for (UINT i = 0; i < 4; ++i) 
		m_startPoint[i] = point[i];
	
	m_doorRect[0].left			= point[0].xPos;
	m_doorRect[0].top			= point[0].zPos + 30.0f;
	m_doorRect[0].right		= point[0].xPos + 30.0f;
	m_doorRect[0].bottom	= point[0].zPos - 30.0f;

	m_doorRect[1].left			= point[1].xPos - 30.0f;
	m_doorRect[1].top			= point[1].zPos + 30.0f;
	m_doorRect[1].right		= point[1].xPos;
	m_doorRect[1].bottom	= point[1].zPos - 30.0f;

	m_doorRect[2].left			= point[2].xPos - 30.0f;
	m_doorRect[2].top			= point[2].zPos;
	m_doorRect[2].right		= point[2].xPos + 30.0f;
	m_doorRect[2].bottom	= point[2].zPos - 30.0f;

	m_doorRect[3].left			= point[3].xPos - 30.0f;
	m_doorRect[3].top			= point[3].zPos + 30.0f;
	m_doorRect[3].right		= point[3].xPos + 30.0f;
	m_doorRect[3].bottom	= point[3].zPos;
}

Point* Room::RegistShader(BasePhysX * phys, bool state, const char& loc)
{
	if (state) {
		if(m_enemyShader)
			m_enemyShader->setPhys(phys);
		if(m_mapShader)
			m_mapShader->SetPhys(phys);

		return &m_startPoint[loc];
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
	//���� �ִϸ��̼��� ���⿡ �ִϸ���Ʈ X
	//m_mapShader->Animate(fTime);

	//�� �̵��Լ�
	if (isClear) { //���� ���� Ŭ��������� ��쿡�� ���� ������ �̵��� �� ����
		for (UINT i = 0; i < 4; ++i) {
			if (playerPos.x > m_doorRect[i].left && playerPos.x < m_doorRect[i].right &&
				playerPos.z < m_doorRect[i].top && playerPos.z > m_doorRect[i].bottom) {
				switch (i) { //�� �տ� �浹������ �ΰ� �浹üũ, ������ �浹�� ���� �� �̵�
				case START_EAST: //���� ��ѹ�, ��������, ���� �ٲٶ�� bool��
					change = Door(1, START_WEST, true);
					break;
				case START_WEST:
					change = Door(1, START_EAST, true);
					break;
				case START_SOUTH:
					change = Door(1, START_NORTH, true);
					break;
				case START_NORTH:
					change = Door(1, START_SOUTH, true);
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