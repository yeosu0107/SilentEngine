#pragma once
#include "D3DUtil.h"
#include "UploadBuffer.h"

struct RenderObject
{
	RenderObject() = default;

	XMFLOAT4X4				m_World = D3DMath::Identity4x4();
	XMFLOAT4X4				m_TexTransform = D3DMath::Identity4x4();

	UINT					m_ObjCBIndex = -1;

	Material*				m_pMaterials;

	vector<MeshGeometry*>	m_ppMeshes;
	UINT					m_iMeshes;

	D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT					m_iIndexCount = 0;
	
	UINT					m_iStartIndexLocation = 0;
	int						m_iBaseVertexLocation = 0;
};

class GameObjects
{
public:
	GameObjects(int nMeshes);
	~GameObjects();

public:
	void SetMesh(int nIndex, MeshGeometry* pMesh);
	void SetMaterial(Material* pMaterial);
	

private:
	shared_ptr<UploadBuffer<ObjectConstants>>		m_ObjectCB = nullptr;
	unique_ptr<RenderObject>						m_pRenderItem = nullptr;

	
};

