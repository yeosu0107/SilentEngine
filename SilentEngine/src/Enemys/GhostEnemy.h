#pragma once

#include "..\Object\Enemy.h"

class Ghost : public Enemy
{
private:
public:
	Ghost(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Ghost();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	virtual void Attack();
	virtual void Skill() {}
	virtual void Hitted(int damage);
	virtual void Hitted(DamageVal& damage);
	virtual void Death();

	virtual void Animate(float fTime);
};

class GhostAI : public BaseAI
{
private:
	int attack_timer = 0;
	float prevAngle = 0.0f;
public:
	GhostAI(GameObject* tmp);
	~GhostAI() {}

	virtual void idleState();
	virtual void patrolState();
	virtual void attackState();
	virtual void hittedState();
	virtual void deathState();
};