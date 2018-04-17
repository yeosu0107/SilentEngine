#pragma once

#include "..\Shaders\Shaders.h"
#include "ModelObject.h"

class ModelShader : public ObjectShader
{
protected:
	UINT				modelIndex;

public:
	ModelShader();
	ModelShader(UINT index);
	~ModelShader();

	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout(int index = 0);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets = 1, void * pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	virtual void RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera, XMFLOAT3& cameraPos, float offset);
	virtual void Animate(float fTimeElapsed);

	virtual void setPhys(BasePhysX* phys) {}
	virtual void releasePhys() {}
	virtual GameObject** getObjects(UINT& num) { return nullptr; }
};

class DynamicModelShader : public ModelShader
{
protected:
	unique_ptr<UploadBuffer<CB_DYNAMICOBJECT_INFO>>	m_BoneCB = nullptr;
	UploadBuffer<LIGHTS>*							m_LightsCB = nullptr;
	UploadBuffer<MATERIALS>*						m_MatCB = nullptr;

public:
	DynamicModelShader(int index);
	~DynamicModelShader();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device * pd3dDevice);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets = 1, void * pContext = NULL);
	virtual void Animate(float fTimeElapsed);

	virtual void SetLightsUploadBuffer(UploadBuffer<LIGHTS>* pLightBuf) { m_LightsCB = pLightBuf; }
	virtual void SetMaterialUploadBuffer(UploadBuffer<MATERIALS>* pMatBuf) { m_MatCB = pMatBuf; }

	virtual GameObject* getObject(int index) { return m_ppObjects[index]; }
	virtual GameObject** getObjects(UINT& num) { 
		num = m_nObjects;
		return m_ppObjects.data(); 
	}
};