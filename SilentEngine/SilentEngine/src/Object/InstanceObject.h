#pragma once
#include "D3DUtil.h"
#include "UploadBuffer.h"
#include "Camera.h"
#include "Mesh.h"

class GameObject;

class InstanceObject : public GameObject
{
public:
	InstanceObject();
	~InstanceObject();

public:
	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void SetInstanceDataResource(ComPtr<ID3D12Resource> pResource);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera = NULL);
	virtual void SetInstanceCount(UINT nInstanceCount);

private:
	UINT					m_nInstanceCount = 0;
	ComPtr<ID3D12Resource>	m_pInstanceDataResource = nullptr;
	
};

