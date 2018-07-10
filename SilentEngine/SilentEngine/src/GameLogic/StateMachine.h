#pragma once

#include "Status.h"

enum STATE {
	idle = 0,		//기본상태
	tracking,	//추적상태
	patrol,		//순찰상태
	attack,		//공격상태
	skill,			//스킬상태
	avoid,		//회피/도망 상태
	hitted,		//공격받은상태
	death,			//죽음상태
	//플레이서 추가 상태
	attack2,
	attack3,
	kick,
	upper
};

class StateMachine
{
protected:
	Status* m_status = nullptr;

	STATE m_state;
	float fTimeElapsed;

	void (StateMachine::*m_stateFunc[8])();
public:
	StateMachine() : m_state(STATE::idle) {};
	~StateMachine() {}

	virtual void update(float fTime) {
		fTimeElapsed = fTime;
		if (m_state > 7)
			m_state = STATE::idle;
		(this->*m_stateFunc[m_state])();
	}
	virtual void changeState(STATE newState) {
		m_state = newState;
	}

	virtual void setFunc() {
		m_stateFunc[0] = &StateMachine::idleState;
		m_stateFunc[1] = &StateMachine::trackingState;
		m_stateFunc[2] = &StateMachine::patrolState;
		m_stateFunc[3] = &StateMachine::attackState;
		m_stateFunc[4] = &StateMachine::skillState;
		m_stateFunc[5] = &StateMachine::avoidState;
		m_stateFunc[6] = &StateMachine::hittedState;
		m_stateFunc[7] = &StateMachine::deathState;
	}
	virtual void idleState() = 0;
	virtual void trackingState() = 0;
	virtual void patrolState() = 0;
	virtual void attackState() = 0;
	virtual void skillState() = 0;
	virtual void avoidState() = 0;
	virtual void hittedState() = 0;
	virtual void deathState() = 0;

	Status* getStatus() { return m_status; }
	STATE getState() const { return m_state; }
};

class BaseAI : public StateMachine
{
protected:
	GameObject*			m_owner;

	float						m_range;
	bool						m_melee;

	UINT						m_patrolTimer = 0;
	float						m_personalRange;
public:
	BaseAI(GameObject* tmp);
	~BaseAI() {}

	virtual void idleState();
	virtual void trackingState();
	virtual void patrolState();
	virtual void attackState();
	virtual void skillState();
	virtual void avoidState();
	virtual void hittedState();
	virtual void deathState();

	void setValue(UINT hp, UINT attack, UINT move, float range, float personal, bool agg);

	bool recognize(XMFLOAT3& pos, float range);
	XMFLOAT3 trackDir(XMFLOAT3& pos);
	float rotDir(XMFLOAT3& pos);

	void Death();
};