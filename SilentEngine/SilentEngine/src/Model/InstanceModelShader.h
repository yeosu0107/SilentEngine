#pragma once
#include "ModelShader.h"

class InstanceModelShader : public ModelShader
{
private:
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
};