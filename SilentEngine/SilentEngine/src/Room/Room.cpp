#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), m_enemyShader(nullptr), m_mapShader(nullptr)
{
}

Room::~Room()
{
	//���̴��� delete�� scene���� �ѹ��� ó��
	//���⼭ ����
}

void Room::RegistShader(BasePhysX * phys, bool state)
{
	if (state) {
		m_enemyShader->setPhys(phys);
		m_mapShader->SetPhys(phys);
	}
	else {

	}
}

