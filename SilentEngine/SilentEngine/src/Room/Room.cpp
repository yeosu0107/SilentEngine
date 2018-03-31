#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), 
	m_enemyShader(nullptr), m_mapShader(nullptr), isEnemy(false)
{
}

Room::~Room()
{
	//���̴��� delete�� scene���� �ѹ��� ó��
	//���⼭ ����
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

		return nullptr;
	}
}

