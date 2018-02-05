#include "stdafx.h"
#include "tMesh.h"

tMesh::tMesh(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList,
	vector<vertex>& vertices, vector<int>& indices)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = (int)vertices.size();
	m_nIndices = (int)indices.size();

	m_nStride = sizeof(vertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//버텍스 버퍼
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, vertices.data(), m_nStride * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	//인덱스 버퍼
	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, indices.data(), sizeof(int)*m_nIndices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT)*m_nIndices;
}

tMesh::~tMesh()
{
}


void tMesh::StartAnimation(string aName)
{
	m_currAnimation = &m_Animation[aName];
	m_currAnimation->Start();
}

void tMesh::StopAllAnimation()
{
	for (auto& p : m_Animation) {
		p.second.Stop();
	}
}

UINT tMesh::getCurrentAnimPos(XMFLOAT4X4 * offset)
{
	if (m_currAnimation)
	{
		m_currAnimation->GetCurrentOffsets(offset);
		return m_currAnimation->GetNumBones();
	}

	return 0;
}

int tMesh::getNumBone(string aName)
{
	for (UINT i = 0; i<m_Bones.size(); i++)
	{
		if (aName == m_Bones[i].name)
			return i;
	}

	return -1;
}

tBone * tMesh::getBone(string aName)
{
	for (UINT i = 0; i<m_Bones.size(); i++)
	{
		if (aName == m_Bones[i].name)
			return &m_Bones[i];
	}

	return nullptr;
}

void tMesh::Tick(float dt)
{
	if (m_currAnimation)
		m_currAnimation->Tick(dt);
}
