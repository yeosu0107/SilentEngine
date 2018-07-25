#pragma once

#include "..\Object\Enemy.h"

class Skull : public Enemy
{
private:
public:
	Skull(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Skull();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	virtual void Skill();
};