#pragma once
#include "Shaders.h"
#include "UIObject.h"

#define NUM_MAX_UITEXTURE 4
#define NUM_MAXIMUMNUMBER_LENGTH 8

enum UINumberType { NUM_TYPE_FLOAT_NONE, NUM_TYPE_FLOAT_DIVISION, NUM_TYPE_FLOAT_PERCENTAGE, NUM_TYPE_UINT_NONE, NUM_TYPE_UINT_DIVISION, NUM_TYPE_UINT_PERCENTAGE };

struct CB_NUMBER_INFO {
	UINT m_Type = 0.0f; // 0 : float, 1 : float 분수 , 2 : float 퍼센트 , 3 : int , 4 : int 분수 , 5 : int 퍼센트
	UINT m_MaximumLength; // 문자 최대 길이
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
	virtual void SetNowSprite(XMUINT2& nowSprite, UINT index = 0);	// 현재 스프라이트 수정
	virtual void SetPosScreenRatio(XMFLOAT2& ratio, UINT index = 0);	// 위치를 스크린 비율로 조정
	virtual void SetPosScreenRatio(XMFLOAT2& ratio, const OPTIONSETALL);	// 위치를 스크린 비율로 조정
	virtual void SetAlpha(float alpha, UINT index = 0);			// ALpha값 수정
	virtual void SetEnable(bool enable, UINT index = 0) { m_pUIObjects[index]->m_bEnabled = enable; }	// 시야에 보일지 확인
	virtual void SetScale(XMFLOAT2* scale, UINT index = 0) { m_pUIObjects[index]->m_xmf2Scale = *scale; }
	virtual void SetScale(XMFLOAT2* scale, const OPTIONSETALL);	// 이 셰이더의 모든 오브젝트 Scale변경
	virtual UIObject* getObejct(UINT index) { return m_pUIObjects[index]; }

	virtual void MovePos(XMFLOAT2& pos, UINT index = 0);	// 현재 위치 기준으로 pos 픽셀만큼 이동 
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateCollisionBox();	// 버튼 충돌 박스생성
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int index = 0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);	
	virtual void SetTimer(float apearTime, float existTime, float retreatTime);							// 등장 시간 Set
	virtual XMUINT2 GetSpriteSize(const int texIndex, CTexture* pTexture, XMUINT2& numSprite = XMUINT2(1, 1));

protected:
	unique_ptr<UploadBuffer<CB_UI_INFO>>	m_ObjectCB = nullptr;
	std::vector<UIObject*>					m_pUIObjects;

	bool									m_bTimeUI;
	float									m_ApearTime;	// 등장하는 시간
	float									m_RetreatTime;	// 사라지는 시간
	float									m_ExistTime;	// 존재하는 시간
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
	vector<DWORD> m_pCooldown;		// 쿨타임
	vector<DWORD*> m_pSkillTime;	// 현재까지 경과된 시간
	vector<DWORD*> m_pBonus;	// 현재까지 경과된 시간
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