#pragma once
#include "StateMachine.h"

class PlayerLogic : public  StateMachine
{
private:
	GameObject*		m_owner;
	UINT					m_attackIndex;
	bool					m_isNextIndex = false;
	const UINT			m_maxAttackIndex = 3;
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
	UINT getAttackIndex() const { return m_attackIndex; }
};