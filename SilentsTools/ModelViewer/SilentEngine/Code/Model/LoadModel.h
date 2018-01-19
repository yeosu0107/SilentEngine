#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "..\Object\Mesh\Mesh.h"
#include "..\Object\Object.h"

using namespace std;

struct vertexData
{
#define BONES_PER_VERTEX 4

	XMFLOAT3	m_pos;
	XMFLOAT3	m_normal;
	XMFLOAT2	m_tex;
	XMFLOAT3	m_weights;
	BYTE			m_bornIndex[BONES_PER_VERTEX];

	vertexData() {}
	vertexData(XMFLOAT3& pos, XMFLOAT3& normal, XMFLOAT2& tex) : 
	m_pos(pos), m_normal(normal), m_tex(tex)
	{
		m_weights = XMFLOAT3(0.0f, 0.0f, 0.0f);
		memset(m_bornIndex, 0, sizeof(BYTE) * BONES_PER_VERTEX);
	}

	void AddBoneData(UINT index, float weight) {
		if (m_weights.x == 0.0f) {
			m_bornIndex[0] = (BYTE)index;
			m_weights.x = weight;
		}
		else if (m_weights.y == 0.0f) {
			m_bornIndex[1] = (BYTE)index;
			m_weights.y = weight;
		}
		else if(m_weights.z==0.0f) {
			m_bornIndex[2] = (BYTE)index;
			m_weights.z = weight;
		}
		else {
			m_bornIndex[3] = (BYTE)index;
		}
	}
};

struct meshData
{
	vector<vertexData>	m_vertices;
	vector<int>				m_indices;
	UINT						m_materialIndex;

	meshData() {
		m_materialIndex = 0;
	}
};

class LoadModel
{
private:
	const aiScene*		m_pScene;
	vector<meshData>	m_meshes;

	UINT						m_numVertices;
	UINT						m_numMaterial;
public:
	LoadModel(const string& fileName);
	~LoadModel();

	void InitScene();
	void InitMesh(UINT index, const aiMesh* pMesh);
};

