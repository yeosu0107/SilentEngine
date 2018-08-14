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

		//GlobalVal::getInstance()->getPlayer()->Hitted(*m_damageVal);
	}
}

void Rich::Skill()
{
	if (skillKind == 0)
		Meteor();
	else if (skillKind == 1)
		PushBackSkill();
	else {
#ifdef _DEBUG
		printf("Unknown boss skill type\n");
#endif
		Meteor();
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

	if (m_State->getState() == STATE::death)
		return;
	m_damageStack += damage.baseDamage;
	m_status->m_health -= damage.baseDamage;
	m_hitback = damage.hitback;
#ifdef _DEBUG
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
#endif
	if (m_damageStack > 100) {
		ChangeAnimation(EnemyAni::AniHitted);
		m_damageStack = 0;
	}
	else if (m_damageStack > 70) {
		skillKind = 1;
		m_State->changeState(STATE::skill);
		return;
	}
	

	m_State->changeState(STATE::hitted);
}

void Rich::PushBackSkill()
{
	ChangeAnimation(EnemyAni::AniSkill);
	
	//트리거 타임 하드코딩
	if (m_ani[m_AnimIndex]->getAnimTime() > 23) {
		DamageVal* skillval = new DamageVal(4.0f, m_baseDamage*2);
		GlobalVal::getInstance()->getPlayer()->Hitted(*skillval);
		skillKind = 0;
	}
}

void Rich::Meteor()
{
	ChangeAnimation(EnemyAni::AniSkill);
	if (m_loopCheck == LOOP_TRIGGER ||
		m_loopCheck == LOOP_STOP) {
		avoid = true;
		if (bulletTime<100)
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
