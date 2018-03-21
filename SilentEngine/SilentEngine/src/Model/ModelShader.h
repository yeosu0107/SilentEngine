#pragma once

#include "..\Shaders\Shaders.h"
#include "ModelObject.h"

class ModelShader : public ObjectShader
{
protected:
	UINT				modelIndex;
	BasePhysX*	globalPhysX;

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

	void setPhysics(BasePhysX* global) {
		globalPhysX = global;
	}
};

class DynamicModelShader : public ModelShader
{
private:
	unique_ptr<UploadBuffer<CB_DYNAMICOBJECT_INFO>>	m_BoneCB = nullptr;
public:
	DynamicModelShader(int index);
	~DynamicModelShader();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device * pd3dDevice);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext);
	virtual void Animate(float fTimeElapsed);

	GameObject* getObject(int index) { return m_ppObjects[index]; }
};