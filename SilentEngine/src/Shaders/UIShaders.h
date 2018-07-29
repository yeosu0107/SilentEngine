#pragma once
#include "Shaders.h"
#include "UIObject.h"

#define NUM_MAX_UITEXTURE 4
#define NUM_MAXIMUMNUMBER_LENGTH 8

enum UINumberType { NUM_TYPE_FLOAT_NONE, NUM_TYPE_FLOAT_DIVISION, NUM_TYPE_FLOAT_PERCENTAGE, NUM_TYPE_UINT_NONE, NUM_TYPE_UINT_DIVISION, NUM_TYPE_UINT_PERCENTAGE };

struct CB_NUMBER_INFO {
	UINT m_Type = 0.0f; // 0 : float, 1 : float �м� , 2 : float �ۼ�Ʈ , 3 : int , 4 : int �м� , 5 : int �ۼ�Ʈ
	UINT m_MaximumLength; // ���� �ִ� ����
	float m_fOriginNumber1;
	float m_fOriginNumber2; 
	UINT m_fNumber[NUM_MAXIMUMNUMBER_LENGTH] = { 1, 2, 3, 4, 5, 6, 7, 8 };
};

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
	virtual UIObject* getObejct(UINT index) { return m_pUIObjects[index]; }

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
	float RemainingCooldown(UINT index);
protected:
	vector<DWORD> m_pCooldown;		// ��Ÿ��
	vector<DWORD*> m_pSkillTime;	// ������� ����� �ð�
	vector<DWORD*> m_pBonus;	// ������� ����� �ð�
};

class NumberUIShaders : public UIShaders
{
	enum NumberIndex { VOIDDIGIT = 10, POINT = 11, DIVISION = 12, PERCENTAGE = 13 };

public:
	NumberUIShaders() {};
	~NumberUIShaders() {};

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device * pd3dDevice);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);

	virtual void SetData(float data1, float data2);
	virtual void LinkData(void* data1, void* data2);
	virtual void LinkEnable(bool* point) { m_pEnabled = point; }

	void SetNumberInfo(UINT type, UINT length);
	void ConvertOptNoneToUINTArray(float data, UINT nLength, bool isFloat);
	void ConvertOptDivisionToUINTArray(float data1, float data2, UINT nLength, bool isFloat);

protected:
	unique_ptr<UploadBuffer<CB_NUMBER_INFO>>	m_NumberInfoCB = nullptr;
	CB_NUMBER_INFO	m_NumberInfo;

	bool* m_pEnabled = nullptr;
	void* m_pData1 = nullptr;
	void* m_pData2 = nullptr;
};