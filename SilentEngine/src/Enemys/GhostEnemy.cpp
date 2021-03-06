#include "stdafx.h"
#include "GhostEnemy.h"
#include "..\Shaders\ProjectileShader.h"

Ghost::Ghost(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	if (m_State)
		delete m_State;
	m_State = new GhostAI(this);
	m_State->setValue(1, 50, 50, 300, 150, false);
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
}



void Ghost::Attack()
{
	ProjectileShader* myProjectile = reinterpret_cast<ProjectileShader*>
		(GlobalVal::getInstance()->getProjectile());

	XMFLOAT3 myPos = GetPosition();
	myPos.y += 20.0f;
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
	playerPos.y += 20.0f;

	myProjectile->Shoot(myPos, playerPos);
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
	//바로 객체 사망 함수를 호출 시 
	//물리객체 콜백함수의 호출과 물리객체 소멸함수의 호출이 동시에 수행되면서
	//에러메시지가 출력
	//m_State->Death();

	if (m_State->getState() == STATE::death)
		return;
	m_status->m_health -= damage.baseDamage;
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
	m_State->changeState(STATE::hitted);
}

void Ghost::Death()
{
	//사망애니메이션 없음
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


void GhostAI::idleState()
{
	changeState(STATE::patrol);
}

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
	if (!m_melee) {
		LookToPlayer();
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

void GhostAI::deathState()
{
	Death();
}