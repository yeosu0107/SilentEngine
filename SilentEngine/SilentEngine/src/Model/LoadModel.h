#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "..\Object\Mesh\Mesh.h"
#include "..\Object\GameObjects.h"

#pragma comment(lib, "lib/assimp.lib")

inline XMMATRIX aiMatrixToXMMatrix(const aiMatrix4x4& offset)
{
	return XMMATRIX(&offset.a1);
}

struct vertexDatas
{
#define BONES_PER_VERTEX 4

	XMFLOAT3	m_pos;
	XMFLOAT3	m_normal;
	XMFLOAT2	m_tex;
	XMUINT4		m_bornIndex;
	XMFLOAT3	m_weights;

	vertexDatas() {}
	vertexDatas(XMFLOAT3& pos, XMFLOAT3& normal, XMFLOAT2& tex) :
		m_pos(pos), m_normal(normal), m_tex(tex)
	{
		m_weights = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_bornIndex = XMUINT4(0, 0, 0, 0);
	}

	void AddBoneData(UINT index, float weight) {
		if (m_weights.x == 0.0f) {
			m_bornIndex.x = index;
			m_weights.x = weight;
		}
		else if (m_weights.y == 0.0f) {
			m_bornIndex.y = index;
			m_weights.y = weight;
		}
		else if (m_weights.z == 0.0f) {
			m_bornIndex.z = index;
			m_weights.z = weight;
		}
		else {
			m_bornIndex.w = index;
		}
	}
};

struct mesh
{
	vector<vertexDatas>	m_vertices;
	vector<int>				m_indices;
	UINT						m_materialIndex;

	mesh() {
		m_materialIndex = 0;
	}
};

struct Bone
{
	XMMATRIX	BoneOffset;
	XMMATRIX FinalTransformation;

	Bone() {
		BoneOffset = XMMatrixIdentity();
		FinalTransformation = XMMatrixIdentity();
	}
};

class ModelMesh : public MeshGeometry
{
public:
	ModelMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		mesh& meshData);
	virtual ~ModelMesh() {}
};

class LoadModel
{
private:
	const aiScene*			m_pScene;				//모델 정보
	vector<mesh>				m_meshes;			//매쉬 정보
	vector<ModelMesh*>	m_ModelMeshes;	//매쉬 정보 리소스(for 랜더링)
	vector<pair<string, Bone>> m_Bones;		//뼈 정보

	UINT							m_numVertices;
	UINT							m_numMaterial;
	UINT							m_numBones;
public:
	LoadModel(const string& fileName);
	~LoadModel();

	void InitScene();
	void InitMesh(UINT index, const aiMesh* pMesh);
	void SetMeshes(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void InitBones(UINT index, const aiMesh* pMesh);

	ModelMesh**	getMeshes() { return m_ModelMeshes.data(); }
	UINT				getNumMesh() const { return (UINT)m_meshes.size(); }
	vector<pair<string, Bone>>* GetBones() { return &m_Bones; }
};