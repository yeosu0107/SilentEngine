#pragma once
#include "D3DUtil.h"
#include "D3DMath.h"
#include "UploadBuffer.h"

class Camera
{
public:
	Camera();
	~Camera();

public:
	virtual void InitCamera(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildDescriptorHeaps(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildConstantBuffers(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pCommandList);
public:
	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
		float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList);
	float AspectRatio() const;

protected:
	const UINT			m_ClientWidth = 800;
	const UINT			m_ClientHeight = 600;
	UINT				m_nCbvSrvDescriptorSize = 0;

	D3D12_VIEWPORT		m_d3dViewport;
	D3D12_RECT			m_d3dRect;

	XMFLOAT4X4			m_xmf4x4View = D3DMath::Identity4x4();
	XMFLOAT4X4			m_xmf4x4Projection = D3DMath::Identity4x4();
	XMFLOAT4X4			m_xmf4x4World = D3DMath::Identity4x4();

	ComPtr<ID3D12DescriptorHeap>				m_SrvDescriptorHeap = nullptr;
	ComPtr<ID3D12RootSignature>					m_RootSignature = nullptr;
	unique_ptr<UploadBuffer<ObjectConstants>>	m_ObjectCB = nullptr;

};

