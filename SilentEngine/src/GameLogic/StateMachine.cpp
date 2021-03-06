#include "stdafx.h"
#include "StateMachine.h"
#include "..\Object\Enemy.h"

BaseAI::BaseAI(GameObject* tmp) : StateMachine(),
	m_owner(tmp)
{
	//m_status = new Status(100, 100, 50);
	
}

void BaseAI::idleState()
{
	if (m_owner == nullptr)
		return;
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
	if (recognize(playerPos, m_personalRange)) {
		changeState(STATE::attack);
		return;
	}
	//if (m_melee) {
	//	changeState(STATE::patrol);
	//	return;
	//}
	m_owner->Idle();

	if (recognize(playerPos, m_range)) {
		changeState(STATE::tracking);
		return;
	}
	changeState(STATE::patrol);
}

void BaseAI::trackingState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	if (!m_melee) {
		if (recognize(playerPos, m_personalRange)) {
			changeState(STATE::attack);
			return;
		}
		if (recognize(playerPos, m_range)) {
			if (GetTickCount() - m_nowCoolTime > m_skillCoolTime) {
				m_nowCoolTime = GetTickCount();
				changeState(STATE::skill);
				return;
			}
			/*if (rand() % 100 > 98) {
				changeState(STATE::skill);
				return;
			}*/
		}
		
	}
	else {
		if (recognize(playerPos, m_personalRange)) {
			if (rand() % 10 < 6)
				changeState(STATE::attack);
			else
				changeState(STATE::skill);
			return;
		}
	}
	LookToPlayer();
	m_owner->Move(fTimeElapsed);		//플레이어 방향으로 이동
}

void BaseAI::patrolState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	m_owner->Move(fTimeElapsed);

	m_patrolTimer += 1;
	if (m_patrolTimer > 120) {
		m_patrolTimer = 0;
		m_owner->Rotate(0, rand()%180, 0);
	}

	if (recognize(playerPos, m_range))
		changeState(STATE::tracking);
}

void BaseAI::attackState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	/*if (m_status->m_health < 50)
		changeState(STATE::skill);*/

	if (!m_melee) {
		LookToPlayer();
	}

	m_owner->Attack();

	if (m_owner->getAnimLoop() == LOOP_END) {
		if (!recognize(playerPos, m_personalRange + 10.0f))
			changeState(STATE::tracking);
	}
}

void BaseAI::skillState()
{
	if (!m_melee) {
		LookToPlayer();
	}

	m_owner->Skill();

	if (m_owner->getAnimLoop() == LOOP_END) {
		//m_nowCoolTime = GetTickCount();
		changeState(STATE::tracking);
	}
}

void BaseAI::avoidState()
{
}

void BaseAI::hittedState()
{
	//m_owner->Hitted();
	if (m_owner->getAnimLoop() == LOOP_END)
		changeState(STATE::idle);
}

void BaseAI::deathState()
{
	m_owner->Death();
	if (m_owner->getAnimLoop() == LOOP_END) {
		Death();
	}
}

void BaseAI::setValue(UINT hp, UINT attack, UINT move,
	float range, float personal, bool agg)
{
	m_range = range;
	m_melee = agg;
	m_personalRange = personal;
	m_status = new Status(hp, attack, move);
	m_owner->SetSpeed(m_status->m_moveSpeed);
}

bool BaseAI::recognize(XMFLOAT3& pos, float local_range)
{
	float dist = Vector3::Length(Vector3::Subtract(m_owner->GetPosition(), pos, false));

	if (dist < local_range) {
		return true;
	}
	return false;
}

XMFLOAT3 BaseAI::trackDir(XMFLOAT3&  pos)
{
	return Vector3::Subtract(m_owner->GetPosition(), pos, true);
}

//float BaseAI::rotDir(XMFLOAT3& pos)
//{
//	//스칼라 삼중적
//	return Vector3::DotProduct(m_owner->GetUp(), 
//		Vector3::CrossProduct(pos, m_owner->GetLook()));
//}

void BaseAI::Death()
{
	m_owner->Death();
	m_owner->SetLive(false);
	reinterpret_cast<Enemy*>(m_owner)->releasePhys();
	*GlobalVal::getInstance()->getRemainEnemy() -= 1;
	//cout << "remain : " << *GlobalVal::getInstance()->getRemainEnemy() << endl;
}

void BaseAI::LookToPlayer()
{
	//플레이어 바라보기
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	XMFLOAT3 track = trackDir(playerPos);
	track = Vector3::DecompVector(track, true, false, true);

	XMFLOAT3 look = Vector3::DecompVector(m_owner->GetLook(), true, false, true);
	float angle = Vector3::Angle(track, look);

	//스칼라 삼중적 회전방향 결정
	if (Vector3::DotProduct(m_owner->GetUp(),
		Vector3::CrossProduct(track, m_owner->GetLook())) > 0)
		angle *= -1;

	//if (rotDir(track) > 0)
	//	angle *= -1;

	m_owner->Rotate(&m_owner->GetUp(), angle);
}
