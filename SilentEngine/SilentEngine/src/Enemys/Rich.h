#pragma once
#include "..\Object\Enemy.h"

class Rich : public Enemy
{
private:
	int bulletTime = 0;
	bool avoid = false;
public:
	Rich(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Rich();

	virtual void SetAnimations(UINT num, LoadAnimation ** tmp);

	virtual void Skill();
	virtual void Hitted();
};