#include "stdafx.h"
#include "PlayerLogic.h"

PlayerLogic::PlayerLogic(GameObject * tmp) :
	m_owner(tmp), m_attackIndex(0)
{
	m_status = new Status(200, 100, 100);
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
	if (m_owner->getAnimLoop() == LOOP_END) {
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
	if (m_owner->getAnimLoop() == LOOP_END) {
		changeState(STATE::idle);
		return;
	}
	m_owner->Skill();
}

void PlayerLogic::avoidState()
{
}

void PlayerLogic::hittedState()
{
	if (m_owner->getAnimLoop() == LOOP_END)
		changeState(STATE::idle);
}

void PlayerLogic::deathState()
{
	if (m_owner->getAnimLoop() == LOOP_END) {
		//m_owner->SetLive(false);
		m_owner->stopAnim(true);
	}
}

void PlayerLogic::changeState(STATE newState)
{
	/*if (m_state == STATE::attack &&
		newState == STATE::attack) {
		m_isNextIndex = true;
	}*/
	if (m_state > STATE::tracking) {
		if (m_owner->getAnimLoop() != LOOP_END)
			return;
	}
	
	m_state = newState;
	switch (m_state) {
	case STATE::idle:
		m_owner->ChangeAnimation(PlayerAni::Idle);
		break;
	case STATE::hitted:
		m_owner->ChangeAnimation(PlayerAni::Hitted);
		break;
	case STATE::death:
		m_owner->ChangeAnimation(PlayerAni::die);
		break;
	case STATE::attack:
		m_owner->ChangeAnimation(PlayerAni::Attack);
		break;
	case STATE::tracking:
		m_owner->ChangeAnimation(PlayerAni::Move);
		break;
	case STATE::skill:
		m_owner->ChangeAnimation(PlayerAni::Skill);
		break;
	default:
		m_owner->ChangeAnimation(PlayerAni::Idle);
		break;
	}
}