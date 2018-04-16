#pragma once

#include "..\Object\Enemy.h"

class Ghost : public Enemy
{
private:
	UINT m_shootDelay;
public:
	Ghost(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Ghost();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	virtual void Attack();
	virtual void Skill() {}
	virtual void Hitted();
	virtual void Death();
};