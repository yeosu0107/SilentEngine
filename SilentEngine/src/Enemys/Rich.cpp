#include "stdafx.h"
#include "Rich.h"
#include "..\Shaders\ProjectileShader.h"

Rich::Rich(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	m_State->setValue(400, 400, 50, 200, 45, false);
	m_State->setCoolTime(10000); //10초마다 메테오 시전
	m_size = XMFLOAT2(1.0f, 10.0f);
	m_status = m_State->getStatus();
	m_triggerSize = XMFLOAT3(30, 30, 30);
	m_damageVal->hitback = 3.5f;
	
	//m_hitback = 1.5f;
	SetScale(0.5f);
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

void Rich::Attack()
{
	ChangeAnimation(EnemyAni::AniAttack);
	if (m_loopCheck == LOOP_TRIGGER) {
		PxTransform tmpTr(m_Controller->getPosition().x,
			m_Controller->getPosition().y,
			m_Controller->getPosition().z);

		tmpTr = tmpTr.transform(PxTransform(XMtoPX(
			Vector3::ScalarProduct(GetLook(), -30, false)
		)));
		m_damageVal->baseDamage = m_baseDamage;
		m_damageVal->randDamage();
		m_attackTrigger->setGlobalPose(tmpTr, true);
	}
}

void Rich::Skill()
{
	ChangeAnimation(EnemyAni::AniSkill);
	if (m_loopCheck == LOOP_TRIGGER ||
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

void Rich::Hitted(int damage)
{
	if (avoid)
		return;

	Enemy::Hitted(damage);
}

void Rich::Hitted(DamageVal & damage)
{
	if (avoid)
		return;

	Enemy::Hitted(damage);
}
