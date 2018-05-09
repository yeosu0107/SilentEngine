#pragma once

#include "..\Object\Enemy.h"



class CreepArm : public Enemy
{
private:
public:
	CreepArm(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~CreepArm();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);
	virtual void Attack();
	virtual void Hitted();
	virtual void Death();
};

class ArmAI : public BaseAI
{
private:
public:
	ArmAI(GameObject* tmp);
	~ArmAI() {}

	virtual void idleState();
	virtual void attackState();
	virtual void hittedState();
};