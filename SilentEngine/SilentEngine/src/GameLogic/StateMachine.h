#pragma once


class StateMachine
{
protected:
	enum STATE {
		idle = 0,		//기본상태
		tracking,	//추적상태
		patrol,		//순찰상태
		attack,		//공격상태
		skill,			//스킬상태
		avoid,		//회피/도망 상태
		death			//죽음상태
	};

	STATE m_state;
	float fTimeElapsed;

	void (StateMachine::*m_stateFunc[7])(void* pContext);
public:
	StateMachine() : m_state(STATE::idle) {};
	~StateMachine() {}

	

	virtual void update(float fTime, void* pContext = nullptr) {
		fTimeElapsed = fTime;
		(this->*m_stateFunc[m_state])(pContext);
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
		m_stateFunc[6] = &StateMachine::deathState;
	}
	virtual void idleState(void* pContext = nullptr) = 0;
	virtual void trackingState(void* pContext = nullptr) = 0;
	virtual void patrolState(void* pContext = nullptr) = 0;
	virtual void attackState(void* pContext = nullptr) = 0;
	virtual void skillState(void* pContext = nullptr) = 0;
	virtual void avoidState(void* pContext = nullptr) = 0;
	virtual void deathState(void* pContext = nullptr) = 0;
};

class BaseAI : public StateMachine
{
private:
	GameObject*			m_owner;

	float						m_range;
	bool						m_aggrasive;

	UINT						m_patrolTimer = 0;
public:
	BaseAI(GameObject* tmp, float range, bool agg);
	~BaseAI() {}


	virtual void idleState(void* pContext = nullptr);
	virtual void trackingState(void* pContext = nullptr);
	virtual void patrolState(void* pContext = nullptr);
	virtual void attackState(void* pContext = nullptr);
	virtual void skillState(void* pContext = nullptr);
	virtual void avoidState(void* pContext = nullptr);
	virtual void deathState(void* pContext = nullptr);

	bool recognize(XMFLOAT3* pos, float range);
	XMFLOAT3 trackDir(XMFLOAT3* pos);
};