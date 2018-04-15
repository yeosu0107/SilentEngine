#include "stdafx.h"
#include "PlayerLogic.h"

PlayerLogic::PlayerLogic(GameObject * tmp) :
	m_owner(tmp), m_attackIndex(0)
{
	m_status = new Status(100, 100, 100);
}

void PlayerLogic::idleState()
{
	if (m_status->m_health <= 0)
		changeState(STATE::death);
}

void PlayerLogic::trackingState()
{
	if (m_status->m_health <= 0)
		changeState(STATE::death);
}

void PlayerLogic::attackState()
{
	if (m_owner->getAnimRoof() == LOOP_END) {
		if (m_isNextIndex) {
			m_attackIndex += 1;
			if (m_attackIndex >= m_maxAttackIndex)
				m_attackIndex = 0;
			m_isNextIndex = false;
		}
		else {
			changeState(STATE::idle);
			return;
		}
	}

	m_owner->Attack();
}

void PlayerLogic::skillState()
{
}

void PlayerLogic::avoidState()
{
}

void PlayerLogic::hittedState()
{
	if (m_owner->getAnimRoof() == LOOP_END)
		changeState(STATE::idle);
}

void PlayerLogic::deathState()
{
}

void PlayerLogic::changeState(STATE newState)
{
	if (m_state == STATE::attack &&
		newState == STATE::attack) {
		m_isNextIndex = true;
	}
	m_state = newState;
}