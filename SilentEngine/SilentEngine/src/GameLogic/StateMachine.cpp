#include "stdafx.h"
#include "StateMachine.h"
#include "..\Object\Enemy.h"

BaseAI::BaseAI(GameObject* tmp, float range, bool agg) : StateMachine(),
	m_owner(tmp), m_range(range), m_aggrasive(agg)
{
	if (m_aggrasive)
		m_personalRange = 32.0f;
	else
		m_personalRange = 60.0f;

	m_status = new Status(100, 100, 50);
	m_owner->SetSpeed(m_status->m_moveSpeed);
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
	if (m_aggrasive) {
		changeState(STATE::patrol);
		return;
	}
	m_owner->Idle();

	if (recognize(playerPos, m_range)) {
		changeState(STATE::tracking);
		return;
	}
}

void BaseAI::trackingState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	if (recognize(playerPos, m_personalRange)) {
		changeState(STATE::attack);
		return;
	}

	XMFLOAT3 track = trackDir(playerPos);

	float angle = Vector3::Angle(track, m_owner->GetLook());

	if (rotDir(track) > 0)
		angle *= -1;

	m_owner->Rotate(&m_owner->GetUp(), angle);
	m_owner->Move(fTimeElapsed);		//플레이어 방향으로 이동
}

void BaseAI::patrolState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	m_owner->Move(fTimeElapsed);

	m_patrolTimer += 1;
	if (m_patrolTimer > 120) {
		m_patrolTimer = 0;
		m_owner->Rotate(0, 90, 0);
	}

	if (recognize(playerPos, m_range))
		changeState(STATE::tracking);
}

void BaseAI::attackState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();

	if (m_status->m_health < 50)
		changeState(STATE::skill);

	m_owner->Attack();

	if(!recognize(playerPos, m_personalRange + 10.0f))
		changeState(STATE::tracking);
}

void BaseAI::skillState()
{
	m_owner->Skill();

	if (m_owner->getAnimRoof() == LOOP_END)
		changeState(STATE::tracking);
}

void BaseAI::avoidState()
{
}

void BaseAI::hittedState()
{
	//m_owner->Hitted();
	if (m_owner->getAnimRoof() == LOOP_END)
		changeState(STATE::idle);
}

void BaseAI::deathState()
{
	m_owner->Death();
	if (m_owner->getAnimRoof() == LOOP_END) {
		m_owner->SetLive(false);
		reinterpret_cast<Enemy*>(m_owner)->releasePhys();
	}
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

float BaseAI::rotDir(XMFLOAT3& pos)
{
	//스칼라 삼중적
	return Vector3::DotProduct(m_owner->GetUp(), 
		Vector3::CrossProduct(pos, m_owner->GetLook()));
}
