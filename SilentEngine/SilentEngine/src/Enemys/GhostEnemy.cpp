#include "stdafx.h"
#include "GhostEnemy.h"
#include "..\Shaders\ProjectileShader.h"

Ghost::Ghost(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	m_shootDelay = 0;
	if (m_State)
		delete m_State;
	m_State = new GhostAI(this);
	m_State->setValue(10, 50, 50, 300, 150, false);
	m_State->setFunc();
	m_status = m_State->getStatus();
	m_size = XMFLOAT2(2.0f, 10.0f);
	SetScale(0.5f);
}

Ghost::~Ghost()
{
}

void Ghost::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	m_ani[EnemyAni::AniIdle]->EnableLoof();
	//m_ani[EnemyAni::AniAttack]->SetAnimSpeed(2.0f);
}



void Ghost::Attack()
{
	//ChangeAnimation(EnemyAni::Attack);


	ProjectileShader* myProjectile = reinterpret_cast<ProjectileShader*>
		(GlobalVal::getInstance()->getProjectile());

	XMFLOAT3 myPos = GetPosition();
	myPos.y += 20.0f;
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
	playerPos.y += 20.0f;

	myProjectile->Shoot(myPos, playerPos);

	//m_shootDelay = 0;

}


void Ghost::Hitted(int damage)
{
	if (m_State->getState() == STATE::death)
		return;
	m_status->m_health -= damage;
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
	m_State->changeState(STATE::hitted);
}

void Ghost::Hitted(DamageVal & damage)
{
	if (m_State->getState() == STATE::death)
		return;
	m_status->m_health -= damage.baseDamage;
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
	m_State->changeState(STATE::hitted);
}

void Ghost::Death()
{
	//ChangeAnimation(EnemyAni::AniMove);
}

void Ghost::Animate(float fTime)
{
	if (m_Crash)
		Rotate(&GetUp(), 90.0f);
	Enemy::Animate(fTime);
}

GhostAI::GhostAI(GameObject * tmp) :
	BaseAI(tmp) { }


void GhostAI::patrolState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	m_owner->Move(fTimeElapsed);
	
	m_patrolTimer += 1;
	if (m_patrolTimer > 200) {
		m_patrolTimer = 0;
		m_owner->Rotate(0, 90, 0);
	}
	if (m_patrolTimer % 100 == 0 &&
		recognize(playerPos, m_personalRange)) {
		prevAngle = 0.0f;
		m_patrolTimer = 0;
		changeState(STATE::attack);
	}
}

void GhostAI::attackState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	if (!m_melee) {
		XMFLOAT3 track = trackDir(playerPos);

		float angle = Vector3::Angle(track, m_owner->GetLook());

		if (rotDir(track) > 0)
			angle *= -1;
		prevAngle += angle;
		m_owner->Rotate(&m_owner->GetUp(), angle);
	}
	attack_timer += 1;

	if (attack_timer < 50)
		return;
	else {
		m_owner->Attack();
		m_owner->Rotate(&m_owner->GetUp(), prevAngle);
		//prevAngle = 0.0f;
		attack_timer = 0;
		changeState(STATE::patrol);
	}
}

void GhostAI::hittedState()
{
	if (m_owner->getAnimLoop() == LOOP_END)
		changeState(STATE::patrol);
}

