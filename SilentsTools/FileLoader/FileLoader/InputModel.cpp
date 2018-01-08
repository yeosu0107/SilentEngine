
#include "stdafx.h"
#include "InputModel.h"

void VertexBoneData::AddBoneData(unsigned int BoneID, float weight)
{
	for (unsigned int i = 0; i < NUM_BONES_PER_VEREX; ++i) {
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = weight;
			return;
		}
	}
}



bool InputModel::LoadAsset(const string & fileName)
{
	Assimp::Importer importer;
	m_NumVertices = 0;
	m_NumBones = 0;

	const aiScene* pScene=importer.ReadFile(fileName.c_str(), 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
		aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	/*
	aiProcess_Triangulate - 비 삼각형 폴리곤으로 만들어진 모델을 삼각형 기반 메쉬로 변환합니다. (쿼드는 삼각형 2개)
	aiProcess_GenSmoothNormals - 원래 모델에 이미 포함되어 있지 않은 경우 정점 법선을 생성합니다.
	aiProcess_FlipUVsv - Y 축을 따라 텍스처 좌표를 뒤집습니다. 이 기능은 데모에 올바르게 사용 된 Quake 모델을 렌더링하는 데 필요합니다.
	aiProcess_JoinIdenticalVertices - 각 꼭지점마다 하나의 사본을 사용하고 필요할 경우 여러 색인에서 참조하십시오. 메모리를 절약하는 데 도움이됩니다.
	*/

	m_Meshes.resize(pScene->mNumMeshes);
	m_nowMeshIndex = 0;

	InitScene(pScene);
	InitMaterial(pScene, fileName);
	
	for (auto& p : m_BoneMapping) {
		cout << p.first <<"\t"<<p.second<< endl;
	}

	for (auto& p : m_Meshes) {
		for (auto& t : p.m_Vertices) {
			printf("%f %f %f\n", t.m_pos.x, t.m_pos.y, t.m_pos.z);
		}
	}

	return true;
}

void InputModel::InitScene(const aiScene * pScene)
{
	m_Meshes.resize(pScene->mNumMeshes);

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	for (int i = 0; i < m_Meshes.size(); ++i) {
		m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Meshes[i].StartVertex = NumVertices;
		m_Meshes[i].StartIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += pScene->mMeshes[i]->mNumFaces * 3;
		
	}
	/*m_Vertices.reserve(NumVertices);
	m_pnIndices.reserve(NumIndices);*/
	//m_Bones.resize(NumVertices);

	for (int i = 0; i < m_Meshes.size(); ++i) {
		const aiMesh* pMesh = pScene->mMeshes[i];
		InitMesh(i, pMesh);
	}

}

void InputModel::InitMesh(unsigned int index, const aiMesh * pMesh)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	m_Meshes[index].m_Vertices.reserve(pMesh->mNumVertices);
	m_Meshes[index].m_pnIndices.reserve(pMesh->mNumFaces * 3);

	for (int i = 0; i < pMesh->mNumVertices; ++i) {
		const aiVector3D* pPos = &(pMesh->mVertices[i]);
		const aiVector3D* pNormal = &(pMesh->mNormals[i]);
		const aiVector3D* pTexCoord = pMesh->HasTextureCoords(0) ? &(pMesh->mTextureCoords[0][i]) : &Zero3D;
		
		vertexData data(XMFLOAT3(pPos->x, pPos->y, pPos->z),
			XMFLOAT2(pTexCoord->x, pTexCoord->y),
			XMFLOAT3(pNormal->x, pNormal->y, pNormal->z));

		m_Meshes[index].m_Vertices.push_back(data);
	}

	InitBones(index, pMesh);

	for (int i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		m_Meshes[index].m_pnIndices.push_back(face.mIndices[0]);
		m_Meshes[index].m_pnIndices.push_back(face.mIndices[1]);
		m_Meshes[index].m_pnIndices.push_back(face.mIndices[2]);
	}

}

void InputModel::InitMaterial(const aiScene * pScene, const string & fileName)
{
	string::size_type slashIndex = fileName.find_last_of("/");
	string Dir;

	if (slashIndex == string::npos)
		Dir = ".";
	else if (slashIndex == 0)
		Dir = "/";
	else
		Dir = fileName.substr(0, slashIndex);

	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
		const aiMaterial* pMat = pScene->mMaterials[i];

		if (pMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString path;
			if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				string fullPath = Dir + "/" + path.data;
				//메터리얼 저장 경로 반환

				//매터리얼 불러오기 부분
#ifdef _DEBUG
				printf("%s\n", fullPath.c_str());
#endif
			}
		}
	}
	
}

void InputModel::InitBones(unsigned int meshIndex, const aiMesh * pMesh)
{
	m_Meshes[meshIndex].m_Bones.resize(pMesh->mNumVertices);
	for (int i = 0; i < pMesh->mNumBones; ++i) {
		int BoneIndex = 0;
		string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			BoneIndex = m_NumBones++;
			BoneInfo bi;
			bi.BoneOffset = convertAIMatrixToXMFloat(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneInfo.push_back(bi);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (unsigned int b = 0; b < pMesh->mBones[i]->mNumWeights; ++b) {
			//unsigned int vertexID=m_Meshes[meshIndex].StartVertex + pMesh->mBones[i]->mWeights[b].mVertexId;
			unsigned int vertexID = pMesh->mBones[i]->mWeights[b].mVertexId;
			float weight = pMesh->mBones[i]->mWeights[b].mWeight;
			m_Meshes[meshIndex].m_Bones[vertexID].AddBoneData(BoneIndex, weight);
		}
	}
}

