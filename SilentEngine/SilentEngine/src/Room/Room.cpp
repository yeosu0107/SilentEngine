#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), 
	m_enemyShader(nullptr), m_mapShader(nullptr), m_Projectile(nullptr),
	isEnemy(false), isProjectile(false)
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
		m_startPoint[i] = point[i];
	}
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

void Room::Animate(float fTime)
{
	//맵은 애니메이션이 없기에 애니메이트 X
	//m_mapShader->Animate(fTime);
	if (isEnemy)
		m_enemyShader->Animate(fTime);
	if (isProjectile)
		m_Projectile->Animate(fTime);
}