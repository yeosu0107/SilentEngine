
#include "stdafx.h"
#include "InputModel.h"




bool InputModel::LoadAsset(const std::string & fileName)
{
	Assimp::Importer importer;

	const aiScene* pScene=importer.ReadFile(fileName.c_str(), 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
		aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	/*
	aiProcess_Triangulate - �� �ﰢ�� ���������� ������� ���� �ﰢ�� ��� �޽��� ��ȯ�մϴ�. (����� �ﰢ�� 2��)
	aiProcess_GenSmoothNormals - ���� �𵨿� �̹� ���ԵǾ� ���� ���� ��� ���� ������ �����մϴ�.
	aiProcess_FlipUVsv - Y ���� ���� �ؽ�ó ��ǥ�� �������ϴ�. �� ����� ���� �ùٸ��� ��� �� Quake ���� �������ϴ� �� �ʿ��մϴ�.
	aiProcess_JoinIdenticalVertices - �� ���������� �ϳ��� �纻�� ����ϰ� �ʿ��� ��� ���� ���ο��� �����Ͻʽÿ�. �޸𸮸� �����ϴ� �� �����̵˴ϴ�.
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
