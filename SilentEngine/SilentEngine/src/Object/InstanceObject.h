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
protected:
};

class EffectInstanceObject : public InstanceObject
{
public:
	EffectInstanceObject();
	~EffectInstanceObject();

	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetEffectCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dEffectCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetEffectCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dEffectCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }

protected:
	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dEffectCbvGPUDescriptorHandle;
};