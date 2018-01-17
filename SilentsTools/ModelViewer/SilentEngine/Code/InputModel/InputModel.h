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

struct CB_OBJECT_INFO
{
	XMFLOAT4X4		m_xmf4x4World;
	XMFLOAT4X4		m_bone[96];
	UINT					m_nMaterial;
};

inline XMFLOAT4X4 convertAIMatrixToXMFloat(aiMatrix4x4 m) {
	XMMATRIX source = XMLoadFloat4x4(&XMFLOAT4X4(&m.a1));
	XMFLOAT4X4 dst;
	XMStoreFloat4x4(&dst, XMMatrixTranspose(source));
	return dst;
}

//inline XMFLOAT4X4 convertAiQuaternionToXmfloat(aiQuaternion q) {
//	XMVECTOR source = XMLoadFloat4(&XMFLOAT4(q.x, q.y, q.z, q.w));
//	XMFLOAT4X4 dst;
//	XMStoreFloat4x4(&dst, XMMatrixRotationQuaternion(source));
//	return dst;
//}

struct vertexData {
#define NUM_BONES_PER_VEREX 4

	XMFLOAT3 m_pos;
	XMFLOAT3 m_normal;
	XMFLOAT2 m_tex;
	XMINT4 m_boneIndex;
	XMFLOAT4 m_weight;

	vertexData() { }

	vertexData(XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& normal) {
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
		m_boneIndex = XMINT4(0, 0, 0, 0);
		m_weight = XMFLOAT4(0, 0, 0, 0);
	}
	void AddBoneData(unsigned int BoneID, float weight) {
		for (unsigned int i = 0; i < NUM_BONES_PER_VEREX; ++i) {
			if (m_weight.x == 0.0) {
				m_boneIndex.x = BoneID;
				m_weight.x = weight;
				return;
			}
			else if (m_weight.y == 0.0) {
				m_boneIndex.y = BoneID;
				m_weight.y = weight;
				return;
			}
			else if (m_weight.z == 0.0) {
				m_boneIndex.z = BoneID;
				m_weight.z = weight;
				return;
			}
			else if(m_weight.w==0.0) {
				m_boneIndex.w = BoneID;
				m_weight.w = weight;
				return;
			}
		}
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
	

	MeshData() {
		StartIndex = 0;
		StartVertex = 0;
		MaterialIndex = 0;
	}

};

class InputModel
{
private:
	const aiScene* m_pScene;
	
	vector<MeshData> m_Meshes;
	
	vector<BoneInfo> m_BoneInfo;
	map<string, unsigned int> m_BoneMapping; //ª¿¿Ã∏ß∞˙ ¿Œµ¶Ω∫ ∏≈«Œ

	int m_NumVertices;
	int m_NumBones;
	XMFLOAT4X4 m_GlobalInverseTransform;
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
	void Animate(float time, vector<XMFLOAT4X4>& bone);
};

class CModelObject : public CGameObject
{
private:
	float m_fRotationSpeed;
	CModelData* model;
	vector<XMFLOAT4X4> m_boneTraform;

	float tmp;
public:
	CModelObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CModelData* data);
	virtual ~CModelObject();

	virtual void Animate(float fTime);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	virtual XMFLOAT4X4* GetBoneData() { return m_boneTraform.data(); }
	virtual int GetBoneNum() const { return m_boneTraform.size(); }

};