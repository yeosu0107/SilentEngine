#pragma once
#include "..\Object\Enemy.h"

class Rich : public Enemy
{
private:
	int bulletTime = 0;
	bool avoid = false;
	UINT skillKind = 0;
public:
	Rich(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Rich();

	virtual void SetAnimations(UINT num, LoadAnimation ** tmp);

	virtual void Attack();
	virtual void Skill();
	virtual void Hitted(int damage);
	virtual void Hitted(DamageVal& damage);

	void PushBackSkill();
	void Meteor();
};