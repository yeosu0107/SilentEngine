#pragma once
#include "ModelShader.h"

class InstanceModelShader : public ModelShader
{
protected:
	vector<ModelObject*> m_InstanceModel;
public:
	InstanceModelShader(UINT index);
	~InstanceModelShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement);
	
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

	virtual void SetLightsUploadBuffer(UploadBuffer<LIGHTS>* pLightBuf) { m_LightsCB = pLightBuf; }
	virtual void SetMaterialUploadBuffer(UploadBuffer<MATERIALS>* pMatBuf) { m_MatCB = pMatBuf; }

	virtual void SetPhys(BasePhysX* phys);
	virtual void releasePhys();
protected:
	UploadBuffer<LIGHTS>*							m_LightsCB = nullptr;
	UploadBuffer<MATERIALS>*						m_MatCB = nullptr;
};

class InstanceDynamicModelShader : public DynamicModelShader
{
protected:
	vector<ModelObject*> m_InstanceModel;
public:
	InstanceDynamicModelShader(int index);
	~InstanceDynamicModelShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
};