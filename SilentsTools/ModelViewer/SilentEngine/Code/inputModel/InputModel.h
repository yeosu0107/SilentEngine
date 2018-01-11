#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "..\Object\Mesh\Mesh.h"
#include "..\Object\Object.h"

#include <map>


#pragma comment(lib, "assimp.lib")

using namespace std;



inline XMFLOAT4X4 convertAIMatrixToXMFloat(aiMatrix4x4 m) {
	XMMATRIX source = XMLoadFloat4x4(&XMFLOAT4X4(&m.a1));
	XMFLOAT4X4 dst;
	XMStoreFloat4x4(&dst, XMMatrixTranspose(source));
	return dst;
}

struct vertexData {
	XMFLOAT3 m_pos;
	XMFLOAT3 m_normal;
	XMFLOAT2 m_tex;
	int m_boneIndex;
	float m_weight;

	vertexData() { }

	vertexData(XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& normal) {
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
	}
	void AddBoneData(unsigned int BoneID, float weight) {
		m_boneIndex = BoneID;
		m_weight = weight;
	}
};

struct BoneInfo {
	XMFLOAT4X4 BoneOffset;
	XMFLOAT4X4 FinalTranformation;

	BoneInfo() {
		XMStoreFloat4x4(&BoneOffset, XMMatrixIdentity());
		XMStoreFloat4x4(&FinalTranformation, XMMatrixIdentity());
	}
};

//struct VertexBoneData {
//#define NUM_BONES_PER_VEREX 4
//	
//	unsigned int IDs;
//	float Weights;
//
//	VertexBoneData() {
//		//memset(IDs, 0, sizeof(unsigned int) * NUM_BONES_PER_VEREX);
//		//memset(Weights, 0.0, sizeof(float) * NUM_BONES_PER_VEREX);
//	}
//
//	void AddBoneData(unsigned int BoneID, float weight);
//};

struct MeshData {
	unsigned int StartVertex;
	unsigned int StartIndex;
	unsigned int MaterialIndex;

	vector<vertexData> m_Vertices;
	vector<int>	 m_pnIndices;
	//vector<VertexBoneData> m_Bones;
	//vector<BoneInfo> m_BoneInfo;
	

	MeshData() {
		StartIndex = 0;
		StartVertex = 0;
		MaterialIndex = 0;
	}

	//vector<vertexData>& returnVertices() { return m_Vertices; }
	//vector<int>& returnIndices() { return m_pnIndices; }
	//vector<VertexBoneData>returnBones() { return m_Bones; }
};

class InputModel
{
private:
	const aiScene* m_pScene;
	
	vector<MeshData> m_Meshes;
	
	vector<BoneInfo> m_BoneInfo;
	map<string, unsigned int> m_BoneMapping; //���̸��� �ε��� ����

	int m_NumVertices;
	int m_NumBones;
public:
	InputModel() {}
	~InputModel() {}

	bool LoadAsset(const string& fileName);
	void InitScene(const aiScene* pScene);
	void InitMesh(unsigned int index, const aiMesh*pMesh);
	void InitMaterial(const aiScene* pScene, const string& fileName);

	void InitBones(unsigned int meshIndex, const aiMesh* pMesh);

	void BoneTransform(float time, vector<XMFLOAT4X4>& transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

	vector<MeshData>& getMeshes() { return m_Meshes; }
};

class ModelMesh : public CMesh
{
public:
	ModelMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		MeshData* meshData);
	virtual ~ModelMesh() {}
};

class CModelData
{
private:
	string m_fileName;
	vector<ModelMesh*> m_ModelMeshes;
	int m_NumOfMeshes;
	InputModel model;
public:
	CModelData(string fileName, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~CModelData();

	ModelMesh** Meshes() { return m_ModelMeshes.data(); }
	int& NumOfMeshes() { return m_NumOfMeshes; }
	void Animate(float time, vector<XMFLOAT4X4> bone);
};

class CModelObject : public CGameObject
{
private:
	float m_fRotationSpeed;
	CModelData* model;
public:
	CModelObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CModelObject();

	virtual void Animate(float fTime);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
};