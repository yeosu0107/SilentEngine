#pragma once
#include "StateMachine.h"

class PlayerLogic : public  StateMachine
{
private:
	enum PlayerAni
	{
		Idle = 0, Move = 1, Attack = 2, Skill = 3, Hitted = 4, die = 5, Attack2 = 6, Attack3 = 7
	};

	GameObject*		m_owner;
	//UINT					m_attackIndex;
	//bool					m_isNextIndex = false;
	//const UINT			m_maxAttackIndex = 3;
public:
	PlayerLogic(GameObject* tmp);
	~PlayerLogic() {}

	void idleState();
	void trackingState();
	void patrolState() { }
	void attackState();
	void skillState();
	void avoidState();
	void hittedState();
	void deathState();

	virtual void changeState(STATE newState);
	//UINT getAttackIndex() const { return m_attackIndex; }
	void reset();
};