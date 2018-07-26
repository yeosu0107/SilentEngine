#pragma once
#include "Shaders.h"
#include "UIObject.h"

#define NUM_MAX_UITEXTURE 4

class UIShaders : public Shaders
{
public:
	UIShaders();
	~UIShaders();

public:
	virtual D3D12_BLEND_DESC			CreateBlendState(int index = 0);
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState(int index = 0);

	virtual void Animate(float fTimeElapsed);
	virtual void RefreshTimer(float fTimeElapsed, UINT index = 0);
	virtual void SetPos(XMFLOAT2* pos, UINT index = 0) { m_pUIObjects[index]->m_xmf2ScreenPos = *pos; }	
	virtual void SetNowSprite(XMUINT2& nowSprite, UINT index = 0);	// ���� ��������Ʈ ����
	virtual void SetPosScreenRatio(XMFLOAT2& ratio, UINT index = 0);	// ��ġ�� ��ũ�� ������ ����
	virtual void SetPosScreenRatio(XMFLOAT2& ratio, const OPTIONSETALL);	// ��ġ�� ��ũ�� ������ ����
	virtual void SetAlpha(float alpha, UINT index = 0);			// ALpha�� ����
	virtual void SetEnable(bool enable, UINT index = 0) { m_pUIObjects[index]->m_bEnabled = enable; }	// �þ߿� ������ Ȯ��
	virtual void SetScale(XMFLOAT2* scale, UINT index = 0) { m_pUIObjects[index]->m_xmf2Scale = *scale; }
	virtual void SetScale(XMFLOAT2* scale, const OPTIONSETALL);	// �� ���̴��� ��� ������Ʈ Scale����

	virtual void MovePos(XMFLOAT2& pos, UINT index = 0);	// ���� ��ġ �������� pos �ȼ���ŭ �̵� 
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateCollisionBox();	// ��ư �浹 �ڽ�����
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int index = 0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);	
	virtual void SetTimer(float apearTime, float existTime, float retreatTime);							// ���� �ð� Set
	virtual XMUINT2 GetSpriteSize(const int texIndex, CTexture* pTexture, XMUINT2& numSprite = XMUINT2(1, 1));

protected:
	unique_ptr<UploadBuffer<CB_UI_INFO>>	m_ObjectCB = nullptr;
	std::vector<UIObject*>					m_pUIObjects;

	bool									m_bTimeUI;
	float									m_ApearTime;	// �����ϴ� �ð�
	float									m_RetreatTime;	// ������� �ð�
	float									m_ExistTime;	// �����ϴ� �ð�
	float									m_ElapsedTime;
};

class UIHPBarShaders : public UIShaders
{
public:
	UIHPBarShaders() { };
	~UIHPBarShaders() { };

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
};

class UIButtonShaders : public UIShaders
{
public:
	UIButtonShaders() { };
	~UIButtonShaders() { };

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void SetPoint(POINT* pos) { m_pMousePosition = pos; }
	virtual UINT CollisionButton();
	virtual UINT ClickButton();
protected:
	POINT * m_pMousePosition;
};

class UIMiniMapShaders : public UIShaders
{
public:
	UIMiniMapShaders() { };
	~UIMiniMapShaders() { };

public:
	virtual void SetNumObject(int nObject) { m_nObjects = nObject + 1; }
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Animate(float fTimeElapsed);
	virtual void SetNowRoom(UINT* nowRoom) { m_pNowRoom = nowRoom; };
	void setRoomPos(void* data);
private:
	UINT m_pPreRoom = 1;
	UINT* m_pNowRoom;

	CTexture *pTexture = nullptr;
};

class UIFullScreenShaders : public UIShaders
{
public:
	UIFullScreenShaders() {};
	~UIFullScreenShaders() {};

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	//virtual void Animate(float fTimeElapsed);
};

class SkillUIShaders : public UIShaders
{
public:
	SkillUIShaders() {};
	~SkillUIShaders() {};

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Animate(float fTimeElapsed);
	void LinkedSkillTime(DWORD* time, DWORD cooldown, DWORD* bonus, UINT index);

	vector<DWORD> m_pCooldown;		// ��Ÿ��
	vector<DWORD*> m_pSkillTime;	// ������� ����� �ð�
	vector<DWORD*> m_pBonus;	// ������� ����� �ð�
};