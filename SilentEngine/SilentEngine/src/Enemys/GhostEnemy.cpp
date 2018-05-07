#include "stdafx.h"
#include "GhostEnemy.h"
#include "..\Shaders\ProjectileShader.h"

Ghost::Ghost(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	m_shootDelay = 0;
	m_State->setValue(300, 150, false);
	SetScale(0.5f);
}

Ghost::~Ghost()
{
}

void Ghost::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	m_ani[EnemyAni::AniIdle]->EnableLoof();
}



void Ghost::Attack()
{
	//ChangeAnimation(EnemyAni::Attack);

	m_shootDelay += 1;
	if (m_shootDelay % 50 == 0) {
		ProjectileShader* myProjectile = reinterpret_cast<ProjectileShader*>
			(GlobalVal::getInstance()->getProjectile());

		XMFLOAT3 myPos = GetPosition();
		myPos.y += 20.0f;
		XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
		playerPos.y += 20.0f;

		myProjectile->Shoot(myPos, playerPos);

		m_shootDelay = 0;
	}
}


void Ghost::Hitted()
{
	m_status->m_health -= 10;
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
	m_State->changeState(STATE::hitted);
}

void Ghost::Death()
{
}
