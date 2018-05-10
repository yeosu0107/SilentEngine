#include "stdafx.h"
#include "skull.h"
#include "..\Shaders\ProjectileShader.h"

Skull::Skull(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	m_State->setValue(50, 50, 50, 200, 45, false);
	m_size = XMFLOAT2(1.0f, 10.0f);
	m_status = m_State->getStatus();
	SetScale(0.3f);
}

Skull::~Skull()
{
	ModelObject::~ModelObject();
}

void Skull::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
}

void Skull::Skill()
{
	ChangeAnimation(EnemyAni::AniSkill);
	if (m_loopCheck == LOOP_MID) {

		ProjectileShader* myProjectile = reinterpret_cast<ProjectileShader*>
			(GlobalVal::getInstance()->getProjectile());

		XMFLOAT3 myPos = GetPosition();
		//myPos.x += 20.0f;
		//myPos.z += 20.0f;
		myPos.y += 70.0f;
		XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
		playerPos.y += 20.0f;

		myProjectile->Shoot(myPos, playerPos);

	}
}
