#pragma once
#include "Shaders.h"
#include "..\Object\Enemy.h"

class ProjectileShader : public BillboardShader
{
private:
	UINT m_ActiveBullet;
	UINT m_now;

	XMFLOAT3 m_crashes[10];
public:

	ProjectileShader() : BillboardShader() {
		m_ActiveBullet=0;
		m_now = 0;
	};
	~ProjectileShader() {};

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void ReleaseObjects() { }

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void Animate(float fTimeElapsed);

	void Shoot(BasePhysX* phys, XMFLOAT3 myPos, XMFLOAT3 targetPos);
	XMFLOAT3* returnCollisionPos(UINT& num);
};