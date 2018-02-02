#pragma once

#include "D3DUtil.h"
#include "UploadBuffer.h"
#include "D3DMath.h"

struct ObjectConstant
{
	XMFLOAT4X4 xmf4x4World = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4TexTransform = D3DMath::Identity4x4();
	UINT	nMaterialIndex;
	UINT	nObjPad0;	// Vector4를 맞추기 위한 UINT
	UINT	nObjPad1;
	UINT	nObjPad2;
};

struct PassConstants
{
	XMFLOAT4X4 xmf4x4View = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4InvView = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4Proj = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4InvProj = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4ViewProj = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4InvViewProj = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4ViewProjTex = D3DMath::Identity4x4();
	XMFLOAT4X4 xmf4x4ShadowTransform = D3DMath::Identity4x4();

	XMFLOAT3 xmf3EyePosW = { 0.0f, 0.0f, 0.0f };

	float fcbPerObjectPad1 = 0.0f;

	XMFLOAT2 xmf2RenderTargetSize = { 0.0f, 0.0f };
	XMFLOAT2 xmf2InvRenderTargetSize = { 0.0f, 0.0f };

	float fNearZ = 0.0f;
	float fFarZ = 0.0f;
	float fTotalTime = 0.0f;
	float fDeltaTime = 0.0f;

	XMFLOAT4 xmf4AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
};

struct SsaoConstants
{

};

struct MaterialData
{

};

struct Vertex
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT4 xmf2Colors;
	//XMFLOAT3 xmf3Normal;
	//XMFLOAT2 xmf2TexCoord;
	//XMFLOAT3 xmf3TangentU;
};

struct SkinnedVertex
{
	XMFLOAT3 xmf3Pos;
	XMFLOAT3 xmf3Normal;
	XMFLOAT2 xmf2TexCoord;
	XMFLOAT3 xmf3TangentU;
	XMFLOAT3 xmf3BoneWeights;
	BYTE	 bBoneIndices[4];
};

struct FrameResource
{
public:
	
	FrameResource(ID3D12Device* pDevice, UINT nPassCount, UINT nObjectCount, UINT nSkinnedObjectCount, UINT nMaterialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	ComPtr<ID3D12CommandAllocator> m_pCmdListAlloc;

	unique_ptr<UploadBuffer<PassConstants>>		m_pPassCB = nullptr;
	unique_ptr<UploadBuffer<ObjectConstant>>	m_pObjCB = nullptr;
	unique_ptr<UploadBuffer<SsaoConstants>>		m_pSsaoCB = nullptr;
	unique_ptr<UploadBuffer<MaterialData>>		m_pMatsCB = nullptr;
	unique_ptr<UploadBuffer<Vertex>>			m_pVertexCB = nullptr;
	unique_ptr<UploadBuffer<SkinnedVertex>>		m_pSkinVertexCB = nullptr;
};