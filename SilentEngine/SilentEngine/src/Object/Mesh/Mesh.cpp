#include "stdafx.h"
#include "Mesh.h"

MeshGeometry::MeshGeometry(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

MeshGeometry::~MeshGeometry()
{
}

void MeshGeometry::ReleaseUploadBuffers()
{
};

void MeshGeometry::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);			// 입력 조립기 프리미티브 토폴로지 셋
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);	// m_nslot번째 슬롯부터 n개의 view를 통해 버퍼를 사용하게따
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void MeshGeometry::Render(ID3D12GraphicsCommandList * pd3dCommandList, UINT nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);			// 입력 조립기 프리미티브 토폴로지 셋
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);	// m_nslot번째 슬롯부터 n개의 view를 통해 버퍼를 사용하게따
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

/////////////////////////////////////////////////////////////////

MeshGeometryCube::MeshGeometryCube(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, float fWidth, float fHeight, float fDepth) : MeshGeometry(pd3dDevice, pd3dCommandList)
{

	m_nVertices = 8;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	array<CDiffusedVertex, 8> pVertices =
	{
		CDiffusedVertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		CDiffusedVertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		CDiffusedVertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		CDiffusedVertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		CDiffusedVertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		CDiffusedVertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		CDiffusedVertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		CDiffusedVertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	m_nIndices = 36;

	std::array<std::uint16_t, 36> pIndices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	m_pd3dVertexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pVertices.data(), m_nStride * m_nVertices, m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pIndices.data(), sizeof(std::uint16_t) * m_nIndices, m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(std::uint16_t) * m_nIndices;
}

MeshGeometryTextured::MeshGeometryTextured(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, float fWidth, float fHeight, float fDepth) : MeshGeometry(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 8;
	m_nStride = sizeof(CTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float width = fWidth / 2.0f;
	float height = fHeight / 2.0f;
	float depth = fDepth / 2.0f;

	array<CTexturedVertex, 8> pVertices =
	{
		CTexturedVertex({ XMFLOAT3(-width, -height, -depth), XMFLOAT2(0.0f, 1.0f) }),
		CTexturedVertex({ XMFLOAT3(-width, +height, -depth), XMFLOAT2(0.0f, 0.0f) }),
		CTexturedVertex({ XMFLOAT3(+width, +height, -depth), XMFLOAT2(1.0f, 0.0f) }),
		CTexturedVertex({ XMFLOAT3(+width, -height, -depth), XMFLOAT2(1.0f, 1.0f) }),
		CTexturedVertex({ XMFLOAT3(-width, -height, +depth), XMFLOAT2(0.0f, 0.0f) }),
		CTexturedVertex({ XMFLOAT3(-width, +height, +depth), XMFLOAT2(0.0f, 0.0f) }),
		CTexturedVertex({ XMFLOAT3(+width, +height, +depth), XMFLOAT2(1.0f, 0.0f) }),
		CTexturedVertex({ XMFLOAT3(+width, -height, +depth), XMFLOAT2(1.0f, 0.0f) })
	};

	m_nIndices = 36;

	std::array<std::uint16_t, 36> pIndices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	m_pd3dVertexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pVertices.data(), m_nStride * m_nVertices, m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pIndices.data(), sizeof(std::uint16_t) * m_nIndices, m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(std::uint16_t) * m_nIndices;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
MeshGeometryIlluminated::MeshGeometryIlluminated(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : MeshGeometry(pd3dDevice, pd3dCommandList)
{
}

MeshGeometryIlluminated::~MeshGeometryIlluminated()
{
}

void MeshGeometryIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, int nVertices)
{
	int nPrimitives = nVertices / 3;
	UINT nIndex0, nIndex1, nIndex2;
	for (int i = 0; i < nPrimitives; i++)
	{
		nIndex0 = i * 3 + 0;
		nIndex1 = i * 3 + 1;
		nIndex2 = i * 3 + 2;
		XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0], false);
		XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0], false);
		pxmf3Normals[nIndex0] = pxmf3Normals[nIndex1] = pxmf3Normals[nIndex2] = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
	}
}

void MeshGeometryIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, UINT nVertices, UINT *pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices / 3) : (nVertices / 3);
	XMFLOAT3 xmf3SumOfNormal, xmf3Edge01, xmf3Edge02, xmf3Normal;
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = pnIndices[i * 3 + 0];
			nIndex1 = pnIndices[i * 3 + 1];
			nIndex2 = pnIndices[i * 3 + 2];
			if (pnIndices && ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j)))
			{
				xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0], false);
				xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0], false);
				xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, false);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}

void MeshGeometryIlluminated::CalculateTriangleStripVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, UINT nVertices, UINT *pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices - 2) : (nVertices - 2);
	XMFLOAT3 xmf3SumOfNormal(0.0f, 0.0f, 0.0f);
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = ((i % 2) == 0) ? (i + 0) : (i + 1);
			if (pnIndices) nIndex0 = pnIndices[nIndex0];
			nIndex1 = ((i % 2) == 0) ? (i + 1) : (i + 0);
			if (pnIndices) nIndex1 = pnIndices[nIndex1];
			nIndex2 = (pnIndices) ? pnIndices[i + 2] : (i + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0], false);
				XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0], false);
				XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}

void MeshGeometryIlluminated::CalculateVertexNormals(XMFLOAT3 *pxmf3Normals, XMFLOAT3 *pxmf3Positions, int nVertices, UINT *pnIndices, int nIndices)
{
	switch (m_d3dPrimitiveTopology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (pnIndices)
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		else
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices);
		break;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		CalculateTriangleStripVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		break;
	default:
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
MeshGeometryIlluminatedTextured::MeshGeometryIlluminatedTextured(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : MeshGeometryIlluminated(pd3dDevice, pd3dCommandList)
{
}

MeshGeometryIlluminatedTextured::~MeshGeometryIlluminatedTextured()
{
}

//////////////////////////////////////////////////////////////////////////////////
//
MeshGeometryIlluminatedTexturedCube::MeshGeometryIlluminatedTexturedCube(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : MeshGeometryIlluminatedTextured(pd3dDevice, pd3dCommandList)
{
	float xSize = fWidth / 2.0f;
	float ySize = fHeight / 2.0f;
	float zSize = fDepth / 2.0f;

	m_nVertices = 8;
	m_nStride = sizeof(CIlluminatedTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	array<CIlluminatedTexturedVertex, 8> pVertices;

	array<XMFLOAT3, 8> pxmf3Positions =
	{
		XMFLOAT3(-xSize, -ySize, -zSize),
		XMFLOAT3(-xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, -ySize, -zSize),
		XMFLOAT3(-xSize, -ySize, +zSize),
		XMFLOAT3(-xSize, +ySize, +zSize),
		XMFLOAT3(+xSize, +ySize, +zSize),
		XMFLOAT3(+xSize, -ySize, +zSize)
	};

	array<XMFLOAT2, 8> pxmf2TexCoord =
	{
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f)
	};

	m_nIndices = 36;

	std::vector<UINT> pIndices = std::vector<UINT>(36);
	pIndices = 
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	XMFLOAT3 pxmf3Normals[8];
	CalculateVertexNormals(pxmf3Normals, pxmf3Positions.data(), m_nVertices, pIndices.data(), m_nIndices);

	for (int i = 0; i < 8; i++) 
		pVertices[i] = CIlluminatedTexturedVertex(pxmf3Positions[i], pxmf3Normals[i], pxmf2TexCoord[i]);
	
	m_pd3dVertexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pVertices.data(), m_nStride * m_nVertices, m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pIndices.data(), sizeof(UINT) * m_nIndices, m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

MeshGeometryIlluminatedTexturedCube::~MeshGeometryIlluminatedTexturedCube()
{
}

CBoardMeshIlluminatedTextured::CBoardMeshIlluminatedTextured(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, float fWidth, float fHeight, float fDepth)
	: MeshGeometryIlluminated(pd3dDevice, pd3dCommandList)
{
	float xSize = fWidth / 2.0f;
	float ySize = fHeight / 2.0f;
	float zSize = 0.0f;

	m_nVertices = 4;
	m_nStride = sizeof(CTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	array<XMFLOAT3, 4> pxmf3Positions;
	pxmf3Positions =
	{
		XMFLOAT3(-xSize, -ySize, -zSize),
		XMFLOAT3(-xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, -ySize, -zSize)
	};

	array<XMFLOAT2, 4> pxmf2TexCoord;
	pxmf2TexCoord =
	{
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f)
	};

	m_nIndices = 6;

	array<UINT, 6> pIndices;
	pIndices =
	{
		0, 2, 1,
		0, 3, 2
	};

	array<CTexturedVertex, 4> pVertices;

	for (UINT i = 0; i < m_nVertices; ++i)
		pVertices[i] = CTexturedVertex(pxmf3Positions[i], pxmf2TexCoord[i]);

	m_pd3dVertexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pVertices.data(), m_nStride * m_nVertices, m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pIndices.data(), sizeof(UINT) * m_nIndices, m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

CBoardMeshIlluminatedTextured::~CBoardMeshIlluminatedTextured()
{
}



NormalMappingCube::NormalMappingCube(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, float fWidth, float fHeight, float fDepth) : MeshGeometryIlluminated(pd3dDevice, pd3dCommandList)
{
	float xSize = fWidth / 2.0f;
	float ySize = fHeight / 2.0f;
	float zSize = fDepth / 2.0f;

	m_nVertices = 24;
	m_nStride = sizeof(CNormalMapVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	array<XMFLOAT3, 24> pxmf3Positions;
	pxmf3Positions =
	{
		XMFLOAT3(-xSize, -ySize, -zSize),
		XMFLOAT3(-xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, -ySize, -zSize),

		XMFLOAT3(-xSize, -ySize, +zSize),
		XMFLOAT3(+xSize, -ySize, +zSize),
		XMFLOAT3(+xSize, +ySize, +zSize),
		XMFLOAT3(-xSize, +ySize, +zSize),

		XMFLOAT3(-xSize, +ySize, -zSize),
		XMFLOAT3(-xSize, +ySize, +zSize),
		XMFLOAT3(+xSize, +ySize, +zSize),
		XMFLOAT3(+xSize, +ySize, -zSize),

		XMFLOAT3(-xSize, -ySize, -zSize),
		XMFLOAT3(+xSize, -ySize, -zSize),
		XMFLOAT3(+xSize, -ySize, +zSize),
		XMFLOAT3(-xSize, -ySize, +zSize),

		XMFLOAT3(-xSize, -ySize, +zSize),
		XMFLOAT3(-xSize, +ySize, +zSize),
		XMFLOAT3(-xSize, +ySize, -zSize),
		XMFLOAT3(-xSize, -ySize, -zSize),

		XMFLOAT3(+xSize, -ySize, -zSize),
		XMFLOAT3(+xSize, +ySize, -zSize),
		XMFLOAT3(+xSize, +ySize, +zSize),
		XMFLOAT3(+xSize, -ySize, +zSize),
	};

	array<XMFLOAT2, 24> pxmf2TexCoord;
	pxmf2TexCoord =
	{
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),

		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),

		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),

		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),

		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f),

		XMFLOAT2(0.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(1.0f, 0.0f),
		XMFLOAT2(1.0f, 1.0f)
	};

	array<XMFLOAT3, 24> pxmf3Normal;
	pxmf3Normal =
	{
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f),

		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),

		XMFLOAT3(+0.0f, +1.0f, 0.0f),
		XMFLOAT3(+0.0f, +1.0f, 0.0f),
		XMFLOAT3(+0.0f, +1.0f, 0.0f),
		XMFLOAT3(+0.0f, +1.0f, 0.0f),

		XMFLOAT3(+0.0f, -1.0f, 0.0f),
		XMFLOAT3(+0.0f, -1.0f, 0.0f),
		XMFLOAT3(+0.0f, -1.0f, 0.0f),
		XMFLOAT3(+0.0f, -1.0f, 0.0f),

		XMFLOAT3(-1.0f, +0.0f, 0.0f),
		XMFLOAT3(-1.0f, +0.0f, 0.0f),
		XMFLOAT3(-1.0f, +0.0f, 0.0f),
		XMFLOAT3(-1.0f, +0.0f, 0.0f),

		XMFLOAT3(+1.0f, +0.0f, 0.0f),
		XMFLOAT3(+1.0f, +0.0f, 0.0f),
		XMFLOAT3(+1.0f, +0.0f, 0.0f),
		XMFLOAT3(+1.0f, +0.0f, 0.0f)
	};

	m_nIndices = 36;

	array<UINT, 36> pIndices;
	pIndices =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	array<XMFLOAT3, 24> pxmf3Tangent;
	D3DUtil::CalculateTangentArray(m_nVertices, pxmf3Positions.data(), pxmf3Normal.data(), pxmf2TexCoord.data(), m_nVertices / 2, pIndices.data(), pxmf3Tangent.data());

	array<CNormalMapVertex, 24> pVertices;
	for (UINT i = 0; i < m_nVertices; ++i)
		pVertices[i] = CNormalMapVertex(pxmf3Positions[i], pxmf3Normal[i], pxmf2TexCoord[i], pxmf3Tangent[i]);

	m_pd3dVertexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pVertices.data(), m_nStride * m_nVertices, m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = D3DUtil::CreateDefaultBuffer(pd3dDevice, pd3dCommandList, pIndices.data(), sizeof(UINT) * m_nIndices, m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

NormalMappingCube::~NormalMappingCube()
{
}
