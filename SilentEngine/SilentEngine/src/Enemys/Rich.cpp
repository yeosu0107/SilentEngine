#include "stdafx.h"
#include "Rich.h"
#include "..\Shaders\ProjectileShader.h"

Rich::Rich(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	m_State->setValue(50, 50, 50, 200, 45, false);
	m_size = XMFLOAT2(1.0f, 10.0f);
	m_status = m_State->getStatus();
	SetScale(0.3f);
}

Rich::~Rich()
{
	ModelObject::~ModelObject();
}

void Rich::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	
	ModelObject::SetAnimations(num, tmp);
	m_ani[EnemyAni::AniHitted]->SetAnimSpeed(2.0f);
}

void Rich::Skill()
{
	ChangeAnimation(EnemyAni::AniSkill);
	if (m_loopCheck == LOOP_MID ||
		m_loopCheck==LOOP_STOP) {
		avoid = true;
		if(bulletTime<100)
			stopAnim(true);
		else {
			stopAnim(false);
			bulletTime = 0;
			avoid = false;
		}
		bulletTime += 1;

		if (bulletTime % 2 == 0)
			return;
		ProjectileShader* myProjectile = reinterpret_cast<ProjectileShader*>
			(GlobalVal::getInstance()->getProjectile());
		XMFLOAT3 myPos = GetPosition();
		myPos.y += 200.0f;
		XMFLOAT3 startPos;
		XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
		playerPos.y += 20.0f;

		startPos = myPos;
		startPos.x += 100 * (rand() % 10);
		startPos.z += 100 * (rand() % 10);

		myProjectile->Shoot(startPos, playerPos);
	}
	
}

void Rich::Hitted()
{
	if (avoid)
		return;

	Enemy::Hitted();
}
