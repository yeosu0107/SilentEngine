#pragma once
#include "D3DUtil.h"
#include "UploadBuffer.h"
#include "Camera.h"
#include "Mesh.h"

class GameObject;

class EffectInstanceObject : public GameObject
{
public:
	float  m_fAnimationSpeed = 10.0f;
	float	m_fMaxXCount = 0.0f;
	float	m_fNowXCount = 0.0f;
	float	m_fMaxYCount = 0.0f;
	float	m_fNowYCount = 0.0f;

	EffectInstanceObject();
	~EffectInstanceObject();

	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetEffectCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dEffectCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetEffectCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dEffectCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	

	virtual void Animate(float fTimeElapsed);
protected:
	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dEffectCbvGPUDescriptorHandle;
};