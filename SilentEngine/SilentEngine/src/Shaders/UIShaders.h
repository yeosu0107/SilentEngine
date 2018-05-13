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
	virtual void SetPos(XMFLOAT2* pos, UINT index = 0) { m_pUIObjects[index]->m_xmf2ScreenPos = *pos; }
	virtual void SetScale(XMFLOAT2* scale, UINT index = 0) { m_pUIObjects[index]->m_xmf2Scale = *scale; }
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateCollisionBox();
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int index = 0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual XMUINT2 GetSpriteSize(const int texIndex, CTexture* pTexture, XMUINT2& numSprite = XMUINT2(1, 1));

protected:
	unique_ptr<UploadBuffer<CB_UI_INFO>>	m_ObjectCB = nullptr;
	std::vector<UIObject*>					m_pUIObjects;
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