#include "stdafx.h"
#include "tModel.h"

//XMFLOAT4X4 aiMatrix4x42XMFLOAT4X4(aiMatrix4x4 aMatrix)
//{
//	XMMATRIX M = XMLoadFloat4x4(&XMFLOAT4X4(&aMatrix.a1));
//	XMFLOAT4X4 FM;
//	XMStoreFloat4x4(&FM, XMMatrixTranspose(M));
//	return FM;
//}

XMFLOAT3X3 aiMatrix3x32XMFLOAT3X3(aiMatrix3x3 aMatrix)
{
	XMMATRIX M = XMLoadFloat3x3(&XMFLOAT3X3(&aMatrix.a1));
	XMFLOAT3X3 FM;
	XMStoreFloat3x3(&FM, XMMatrixTranspose(M));
	return FM;
	//return XMFLOAT3X3(&aMatrix.a1);
}

tModel::tModel()
{
}

tModel::~tModel()
{
}

bool tModel::LoadFile(const char * fileName)
{
	const aiScene* pScene= aiImportFile(fileName, aiProcess_ConvertToLeftHanded |
		aiProcess_CalcTangentSpace);

	if (pScene) {
		vector<vertex> vertices;
		vector<int> indices;

		for (UINT i = 0; i < pScene->mNumMeshes; ++i) {
			aiMesh* pMesh = pScene->mMeshes[i];

			for (UINT k = 0; k < pMesh->mNumVertices; ++k) {
				vertex tmp;
				tmp.pos = XMFLOAT3(&pMesh->mVertices[k].x);
				tmp.normal = XMFLOAT3(&pMesh->mNormals[k].x);
				tmp.tex = XMFLOAT2(&pMesh->mTextureCoords[0][k].x);
				tmp.index = XMUINT4(100, 100, 100, 100);
				tmp.weight = XMFLOAT3(0.0f, 0.0f, 0.0f);
				vertices.emplace_back(tmp);
			}

			for (UINT j = 0; j < pMesh->mNumFaces; ++j) {
				aiFace pFace = pMesh->mFaces[j];
				indices.emplace_back(pFace.mIndices[0]);
				indices.emplace_back(pFace.mIndices[0]);
				indices.emplace_back(pFace.mIndices[0]);
			}

			if (pMesh->HasBones() && !LoadBones(pScene, pMesh, &vertices))
				return false;
		}
	}
}

bool tModel::LoadBones(const aiScene * pScene, aiMesh * pMesh, vector<vertex>* vertices)
{
	UINT numBones = pMesh->mNumBones;
	vector<string> boneParentName;
	
	/*for (UINT i = 0; i < numBones; ++i) {
		aiBone& boneData = *pMesh->mBones[i];
		aiNode& boneNode = *pScene->mRootNode->FindNode(boneData.mName);

		string boneName = boneData.mName.C_Str();
		XMFLOAT4X4 boneOffset = aiMatrix4x42XMFLOAT4X4(boneData.mOffsetMatrix);
		XMFLOAT4X4 boneToParent = aiMatrix4x42XMFLOAT4X4(boneNode.mTransformation);

		tBone bone = { boneName, boneOffset, boneToParent, 0 };
		m_bones.emplace_back(bone);

		aiNode& parentNode = *boneNode.mParent;
		boneParentName.push_back(parentNode.mName.C_Str());

		for (UINT k = 0; k < boneData.mNumWeights; ++k) {
			UINT index = boneData.mWeights[k].mVertexId;
			vertex& tmp = vertices->at(index);

			if (tmp.weight.x != 0.0f) {
				tmp.weight.x = boneData.mWeights[k].mWeight;

			}
		}
	}*/
	return true;
}

bool tModel::LoadAnimation(const aiScene * pScene)
{
	return false;
}
