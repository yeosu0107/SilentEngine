#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include <iostream>
#include <vector>
#include <map>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#pragma comment(lib, "assimp.lib")

using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

inline XMFLOAT4X4 convertAIMatrixToXMFloat(aiMatrix4x4 m) {
	XMMATRIX source = XMLoadFloat4x4(&XMFLOAT4X4(&m.a1));
	XMFLOAT4X4 dst;
	XMStoreFloat4x4(&dst, XMMatrixTranspose(source));
	return dst;
}

struct vertexData {
	XMFLOAT3 m_pos;
	XMFLOAT2 m_tex;
	XMFLOAT3 m_normal;

	vertexData() { }

	vertexData(XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& normal) {
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
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

struct VertexBoneData {
#define NUM_BONES_PER_VEREX 4
	
	unsigned int IDs[NUM_BONES_PER_VEREX];
	float Weights[NUM_BONES_PER_VEREX];

	VertexBoneData() {
		memset(IDs, 0, sizeof(unsigned int) * NUM_BONES_PER_VEREX);
		memset(Weights, 0.0, sizeof(float) * NUM_BONES_PER_VEREX);
	}

	void AddBoneData(unsigned int BoneID, float weight);
};

struct MeshData {
	unsigned int StartVertex;
	unsigned int StartIndex;
	unsigned int MaterialIndex;

	vector<vertexData> m_Vertices;
	vector<int>	 m_pnIndices;
	vector<VertexBoneData> m_Bones;
	//vector<BoneInfo> m_BoneInfo;
	

	MeshData() {
		StartIndex = 0;
		StartVertex = 0;
		MaterialIndex = 0;
	}

};

class InputModel
{
private:
	vector<MeshData> m_Meshes;
	
	//vector<vertexData> m_Vertices;
	//vector<int>	 m_pnIndices;
	//vector<VertexBoneData> m_Bones;
	vector<BoneInfo> m_BoneInfo;
	map<string, unsigned int> m_BoneMapping; //ª¿¿Ã∏ß∞˙ ¿Œµ¶Ω∫ ∏≈«Œ

	int m_NumVertices;
	int m_NumBones;
	int m_nowMeshIndex;
public:
	InputModel() {}
	~InputModel() {}

	bool LoadAsset(const string& fileName);
	void InitScene(const aiScene* pScene);
	void InitMesh(unsigned int index, const aiMesh*pMesh);
	void InitMaterial(const aiScene* pScene, const string& fileName);

	void InitBones(unsigned int meshIndex, const aiMesh* pMesh);


};

