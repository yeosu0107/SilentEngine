#include "stdafx.h"
#include "Room.h"

Room::Room(UINT type) : m_type(type), m_enemyShader(nullptr), m_mapShader(nullptr)
{
}

Room::~Room()
{
	//셰이더의 delete는 scene에서 한번에 처리
	//여기서 안함
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

