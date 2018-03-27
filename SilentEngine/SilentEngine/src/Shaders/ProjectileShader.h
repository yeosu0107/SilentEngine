#pragma once
#include "Shaders.h"
#include "..\Object\Enemy.h"

class ProjectileShader : public ObjectShader
{
private:
	UINT m_ActiveBullet;
	Bullet**		m_ppBullet;
	UINT m_now;
public:

	ProjectileShader() : ObjectShader() {
		m_ActiveBullet=0;
		m_ppBullet = nullptr;
		m_now = 0;
	};
	~ProjectileShader() {};

public:
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void ReleaseObjects() { }

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void Animate(float fTimeElapsed);

	void Shoot(XMFLOAT3 myPos, XMFLOAT3 targetPos);
};