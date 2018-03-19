#pragma once

#include "..\Shaders\Shaders.h"
#include "ModelObject.h"

class ModelShader : public ObjectShader
{
protected:
	UINT modelIndex;

public:
	ModelShader();
	ModelShader(UINT index);
	~ModelShader();

	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext);
	virtual void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	virtual void Animate(float fTimeElapsed);

};

class DynamicModelShader : public ModelShader
{
private:
	unique_ptr<UploadBuffer<CB_DYNAMICOBJECT_INFO>>	m_BoneCB = nullptr;
public:
	DynamicModelShader(int index);
	~DynamicModelShader();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext);
};