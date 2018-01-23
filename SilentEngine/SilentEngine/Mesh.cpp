#include "stdafx.h"
#include "Mesh.h"

Mesh::MeshData Mesh::CreateBox(float fwidth, float fheight, float fdepth, uint32 numSubdivisions)
{
	MeshData meshData;

	Vertex pVertexData[24];

	float fBoxWidth = 0.5f * fwidth;
	float fBoxHeight = 0.5f * fheight;
	float fBoxDepth = 0.5f * fdepth;

	pVertexData[0] = Vertex(-fBoxWidth, -fBoxHeight, -fBoxDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pVertexData[1] = Vertex(-fBoxWidth, +fBoxHeight, -fBoxDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pVertexData[2] = Vertex(+fBoxWidth, +fBoxHeight, -fBoxDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	pVertexData[3] = Vertex(+fBoxWidth, -fBoxHeight, -fBoxDepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	pVertexData[4] = Vertex(-fBoxWidth, -fBoxHeight, +fBoxDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	pVertexData[5] = Vertex(+fBoxWidth, -fBoxHeight, +fBoxDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pVertexData[6] = Vertex(+fBoxWidth, +fBoxHeight, +fBoxDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pVertexData[7] = Vertex(-fBoxWidth, +fBoxHeight, +fBoxDepth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	pVertexData[8]  = Vertex(-fBoxWidth, +fBoxHeight, -fBoxDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pVertexData[9]  = Vertex(-fBoxWidth, +fBoxHeight, +fBoxDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pVertexData[10] = Vertex(+fBoxWidth, +fBoxHeight, +fBoxDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	pVertexData[11] = Vertex(+fBoxWidth, +fBoxHeight, -fBoxDepth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	pVertexData[12] = Vertex(-fBoxWidth, -fBoxHeight, -fBoxDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	pVertexData[13] = Vertex(+fBoxWidth, -fBoxHeight, -fBoxDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pVertexData[14] = Vertex(+fBoxWidth, -fBoxHeight, +fBoxDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	pVertexData[15] = Vertex(-fBoxWidth, -fBoxHeight, +fBoxDepth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	pVertexData[16] = Vertex(-fBoxWidth, -fBoxHeight, +fBoxDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	pVertexData[17] = Vertex(-fBoxWidth, +fBoxHeight, +fBoxDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	pVertexData[18] = Vertex(-fBoxWidth, +fBoxHeight, -fBoxDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	pVertexData[19] = Vertex(-fBoxWidth, -fBoxHeight, -fBoxDepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	pVertexData[20] = Vertex(+fBoxWidth, -fBoxHeight, -fBoxDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	pVertexData[21] = Vertex(+fBoxWidth, +fBoxHeight, -fBoxDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	pVertexData[22] = Vertex(+fBoxWidth, +fBoxHeight, +fBoxDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	pVertexData[23] = Vertex(+fBoxWidth, -fBoxHeight, +fBoxDepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.m_pVertices.assign(&pVertexData[0], &pVertexData[24]);

	uint32 pIndexData[36];

	pIndexData[0] = 0; pIndexData[1] = 1; pIndexData[2] = 2;
	pIndexData[3] = 0; pIndexData[4] = 2; pIndexData[5] = 3;

	pIndexData[6] = 4; pIndexData[7] = 5; pIndexData[8] = 6;
	pIndexData[9] = 4; pIndexData[10] = 6; pIndexData[11] = 7;

	pIndexData[12] = 8; pIndexData[13] = 9; pIndexData[14] = 10;
	pIndexData[15] = 8; pIndexData[16] = 10; pIndexData[17] = 11;

	pIndexData[18] = 12; pIndexData[19] = 13; pIndexData[20] = 14;
	pIndexData[21] = 12; pIndexData[22] = 14; pIndexData[23] = 15;

	pIndexData[24] = 16; pIndexData[25] = 17; pIndexData[26] = 18;
	pIndexData[27] = 16; pIndexData[28] = 18; pIndexData[29] = 19;

	pIndexData[30] = 20; pIndexData[31] = 21; pIndexData[32] = 22;
	pIndexData[33] = 20; pIndexData[34] = 22; pIndexData[35] = 23;

	meshData.m_pIndices32.assign(&pIndexData[0], &pIndexData[36]);

	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	for (uint32 i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	return meshData;
}

void Mesh::Subdivide(MeshData & meshData)
{
	MeshData inputCopy = meshData;

	meshData.m_pVertices.resize(0);
	meshData.m_pIndices32.resize(0);

	uint32 numTris = (uint32)inputCopy.m_pIndices32.size() / 3;
	for (uint32 i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.m_pVertices[inputCopy.m_pIndices32[i * 3 + 0]];
		Vertex v1 = inputCopy.m_pVertices[inputCopy.m_pIndices32[i * 3 + 1]];
		Vertex v2 = inputCopy.m_pVertices[inputCopy.m_pIndices32[i * 3 + 2]];

		Vertex m0 = MidPoint(v0, v1);
		Vertex m1 = MidPoint(v1, v2);
		Vertex m2 = MidPoint(v0, v2);

		meshData.m_pVertices.push_back(v0); // 0
		meshData.m_pVertices.push_back(v1); // 1
		meshData.m_pVertices.push_back(v2); // 2
		meshData.m_pVertices.push_back(m0); // 3
		meshData.m_pVertices.push_back(m1); // 4
		meshData.m_pVertices.push_back(m2); // 5

		meshData.m_pIndices32.push_back(i * 6 + 0);
		meshData.m_pIndices32.push_back(i * 6 + 3);
		meshData.m_pIndices32.push_back(i * 6 + 5);

		meshData.m_pIndices32.push_back(i * 6 + 3);
		meshData.m_pIndices32.push_back(i * 6 + 4);
		meshData.m_pIndices32.push_back(i * 6 + 5);

		meshData.m_pIndices32.push_back(i * 6 + 5);
		meshData.m_pIndices32.push_back(i * 6 + 4);
		meshData.m_pIndices32.push_back(i * 6 + 2);

		meshData.m_pIndices32.push_back(i * 6 + 3);
		meshData.m_pIndices32.push_back(i * 6 + 1);
		meshData.m_pIndices32.push_back(i * 6 + 4);
	}
}
