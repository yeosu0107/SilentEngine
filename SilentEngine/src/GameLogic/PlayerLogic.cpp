#include "stdafx.h"
#include "PlayerLogic.h"

PlayerLogic::PlayerLogic(GameObject * tmp) :
	m_owner(tmp)
{
	//m_status = new Status(200, 100, 80);
	m_status = new Status(200, 100, 100, 80);
	m_attackType = AttackType::Normal;
}

void PlayerLogic::idleState()
{
	//m_owner->Idle();
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
			changeState(STATE::idle);
			return;

	}

	if (m_attackType == AttackType::Kick) {
		m_owner->Attack_Kick();
	}
	else if (m_attackType == AttackType::Upper) {
		m_owner->Attack_Upper();
	}
	else if (m_attackType == AttackType::Punch) {
		m_owner->Attack_Power();
	}
	else {
		//기본공격
		m_owner->Attack_Normal();
	}
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
	//m_owner->Idle();
	if (m_owner->getAnimLoop() == LOOP_END) {
		//m_owner->SetLive(false);
		
		m_owner->stopAnim(true);
		m_owner->SetLive(false);
	}
}

void PlayerLogic::changeState(STATE newState)
{
	if (newState == STATE::kick) {
		m_attackType = AttackType::Kick;
		m_state = STATE::attack;
		SoundMgr::getInstance()->play(SOUND::SKILL01, CHANNEL::PLAYER);
		m_owner->ChangeAnimation(PlayerAni::KickAttack);
		return;
	}
	if (newState == STATE::upper) {
		m_attackType = AttackType::Upper;
		m_state = STATE::attack;
		SoundMgr::getInstance()->play(SOUND::SKILL02, CHANNEL::PLAYER);
		m_owner->ChangeAnimation(PlayerAni::KickAttack2);
		return;
	}
	if (newState == STATE::punch) {
		m_attackType = AttackType::Punch;
		m_state = STATE::attack;
		SoundMgr::getInstance()->play(SOUND::SKILL03, CHANNEL::PLAYER);
		m_owner->ChangeAnimation(PlayerAni::PowerPunch);
		return;
	}
	/*if (newState == STATE::hitted) {
		m_state = STATE::hitted;
		m_owner->ChangeAnimation(PlayerAni::Hitted);
		return;
	}*/

	if (m_state > STATE::tracking) {
		if (m_state == STATE::attack || m_state == STATE::attack2 || m_state == STATE::attack3 ||
			m_state == STATE::kick || m_state == STATE::punch) {
			if (m_owner->getAnimLoop() == LOOP_TRIGGER || m_owner->getAnimLoop() == LOOP_IN)
				return;
		}

		else if (m_owner->getAnimLoop() != LOOP_END)
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
		SoundMgr::getInstance()->ch_stop(CHANNEL::BGM);
		SoundMgr::getInstance()->play(SOUND::DIE, CHANNEL::FX);
		m_owner->ChangeAnimation(PlayerAni::die);
		break;
	case STATE::attack:
		m_attackType = AttackType::Normal;
		SoundMgr::getInstance()->play(SOUND::ATT01, CHANNEL::PLAYER);
		SoundMgr::getInstance()->play(SOUND::WIND01, CHANNEL::PLAYER);
		m_owner->ChangeAnimation(PlayerAni::Attack);
		break;
	case STATE::attack2:
		m_attackType = AttackType::Normal;
		SoundMgr::getInstance()->play(SOUND::ATT02, CHANNEL::PLAYER);
		SoundMgr::getInstance()->play(SOUND::WIND01, CHANNEL::PLAYER);
		m_state = STATE::attack;
		m_owner->ChangeAnimation(PlayerAni::Attack2);
		break;
	case STATE::attack3:
		m_attackType = AttackType::Normal;
		SoundMgr::getInstance()->play(SOUND::ATT03, CHANNEL::PLAYER);
		SoundMgr::getInstance()->play(SOUND::WIND02, CHANNEL::PLAYER);
		m_state = STATE::attack;
		m_owner->ChangeAnimation(PlayerAni::Attack3);
		break;
		/*case STATE::kick:
			m_owner->ChangeAnimation(PlayerAni::KickAttack);
			m_state = STATE::attack;
			break;*/
	case STATE::tracking:
		m_owner->ChangeAnimation(PlayerAni::Move);
		break;
	case STATE::skill:
		SoundMgr::getInstance()->play(SOUND::SKILL04, CHANNEL::PLAYER);
		m_owner->ChangeAnimation(PlayerAni::Skill);
		break;
	default:
		m_owner->ChangeAnimation(PlayerAni::Idle);
		break;
	}
}

void PlayerLogic::reset()
{
	m_state = STATE::idle;
}
