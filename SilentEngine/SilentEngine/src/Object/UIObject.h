#pragma once
#include "Player.h"

struct CB_UI_INFO {
	XMFLOAT2  m_xmf2ScreenPos;
	XMFLOAT2  m_xmf2ScreenSize;

	XMUINT2	 m_nNumSprite;
	XMUINT2	 m_nNowSprite;

	XMUINT2  m_nSize;
	UINT	 m_nTexType;
	float	 m_fData;

	XMFLOAT2 m_xmf2Scale;
	float    m_fdump1 = 0;
	float    m_fdump2 = 0;
};

class UIObject
{
public:
	UIObject();
	~UIObject();

public:
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }

	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

	virtual void Update(float fTimeElapsed) {};

	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }

	virtual void SetScreenSize(XMFLOAT2& size);
	virtual void SetPosition(XMFLOAT2& pos);
	virtual void SetScale(XMFLOAT2& scale);
	virtual void SetSize(XMUINT2& size);
	virtual void SetNumSprite(XMUINT2& numSprite, XMUINT2& nowSprite);

	virtual void SetType(UINT type) { m_nTexType = type; }

public:
	
	bool										m_bEnabled;
	float										m_fAnimationTime;
	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dCbvGPUDescriptorHandle;

	XMFLOAT2	m_xmf2ScreenPos;
	XMFLOAT2	m_xmf2ScreenSize;

	XMUINT2		m_nNumSprite;
	XMUINT2		m_nNowSprite;

	XMUINT2		m_nSize;
	UINT		m_nTexType;
	float		m_fData = 0.0f;

	XMFLOAT2	m_xmf2Scale = XMFLOAT2(1.0f, 1.0f);
};

class HPBarObject : public UIObject 
{
public:
	HPBarObject() {};
	~HPBarObject() {};

public:
	void SetPlayerStatus(Status* pPlayerStatus);

	virtual void Update(float fTimeElapsed);

protected:
	Status * m_pPlayerStatus;

	float m_fMaxHP;		// �ִ� HP
	float m_fLerpHP;	// 

	float m_fLerpTime;
};