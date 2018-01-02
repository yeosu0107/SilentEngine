
#include "stdafx.h"
#include "InputModel.h"




bool InputModel::LoadAsset(const std::string & fileName)
{
	Assimp::Importer importer;

	const aiScene* pScene=importer.ReadFile(fileName.c_str(), 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
		aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	/*
	aiProcess_Triangulate - 비 삼각형 폴리곤으로 만들어진 모델을 삼각형 기반 메쉬로 변환합니다. (쿼드는 삼각형 2개)
	aiProcess_GenSmoothNormals - 원래 모델에 이미 포함되어 있지 않은 경우 정점 법선을 생성합니다.
	aiProcess_FlipUVsv - Y 축을 따라 텍스처 좌표를 뒤집습니다. 이 기능은 데모에 올바르게 사용 된 Quake 모델을 렌더링하는 데 필요합니다.
	aiProcess_JoinIdenticalVertices - 각 꼭지점마다 하나의 사본을 사용하고 필요할 경우 여러 색인에서 참조하십시오. 메모리를 절약하는 데 도움이됩니다.
	*/

	InitScene(pScene, pScene->mRootNode);

	return true;
}

void InputModel::InitScene(const aiScene * pScene, const struct aiNode* nd)
{
	for (unsigned int index = 0; index < nd->mNumMeshes; ++index) {
		const struct aiMesh* pMesh = pScene->mMeshes[nd->mMeshes[index]];

		InitMesh(pMesh);
	}

	for (unsigned int child = 0; child < nd->mNumChildren; ++child) {
		InitScene(pScene, nd->mChildren[child]);
	}
}

void InputModel::InitMesh(const aiMesh * pMesh)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i) {
		const struct aiFace* face = &pMesh->mFaces[i];
		
		for (unsigned int k = 0; k < face->mNumIndices; ++k) {
			int index = face->mIndices[k];

			const aiVector3D* pPos = &(pMesh->mVertices[index]);
			const aiVector3D* pNormal = &(pMesh->mNormals[index]);
			const aiVector3D* pTexCoord = pMesh->HasTextureCoords(0) ? &(pMesh->mTextureCoords[0][index]) : &Zero3D;

			vertexData data(XMFLOAT3(pPos->x, pPos->y, pPos->z),
				XMFLOAT2(pTexCoord->x, pTexCoord->y),
				XMFLOAT3(pNormal->x, pNormal->y, pNormal->z));

			m_Vertices.emplace_back(data);
		}
	}
}
