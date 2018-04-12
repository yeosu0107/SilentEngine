#include "stdafx.h"
#include "StateMachine.h"
#include "..\Object\Enemy.h"

BaseAI::BaseAI(GameObject* tmp, float range, bool agg) : StateMachine(),
	m_owner(tmp), m_range(range), m_aggrasive(agg)
{

}

void BaseAI::idleState(void * pContext)
{
	if (m_owner == nullptr)
		return;
	if (m_aggrasive) {
		changeState(STATE::patrol);
		return;
	}

	XMFLOAT3* playerPos = reinterpret_cast<XMFLOAT3*>(pContext);

	if (recognize(playerPos, m_range))
		changeState(STATE::tracking);
}

void BaseAI::trackingState(void * pContext)
{
	XMFLOAT3* playerPos = reinterpret_cast<XMFLOAT3*>(pContext);

	m_owner->Move(fTimeElapsed, trackDir(playerPos));		//플레이어 방향으로 이동

	if (recognize(playerPos, 10))
		changeState(STATE::attack);
}

void BaseAI::patrolState(void * pContext)
{
	XMFLOAT3* playerPos = reinterpret_cast<XMFLOAT3*>(pContext);

	m_owner->Move(fTimeElapsed, m_owner->GetLook());

	m_patrolTimer += 1;
	if (m_patrolTimer > 60) {
		m_patrolTimer = 0;
		m_owner->Rotate(0, 90, 0);
	}

	if (recognize(playerPos, m_range))
		changeState(STATE::tracking);
}

void BaseAI::attackState(void * pContext)
{
}

void BaseAI::skillState(void * pContext)
{
}

void BaseAI::avoidState(void * pContext)
{
}

void BaseAI::deathState(void * pContext)
{
}

bool BaseAI::recognize(XMFLOAT3* pos, float local_range)
{
	float dist = Vector3::Length(Vector3::Subtract(m_owner->GetPosition(), *pos, false));

	if (dist < local_range) {
		return true;
	}
	return false;
}

XMFLOAT3 BaseAI::trackDir(XMFLOAT3 * pos)
{
	return Vector3::Subtract(*pos, m_owner->GetPosition(), true);
}
