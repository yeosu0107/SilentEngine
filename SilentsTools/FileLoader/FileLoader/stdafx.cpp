// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// $safeprojectname$.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"

// TODO: 필요한 추가 헤더는
// 이 파일이 아닌 STDAFX.H에서 참조합니다.

void LoadAsset(const char* path)
{
	scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	printf("%d\n", scene->mNumMeshes);

	showMesh(scene, scene->mRootNode);
	LoadBorn(scene, scene->mRootNode);
}
int vertices = 0;
void showMesh(const struct aiScene* sc, const struct aiNode* nd)
{
	for (auto i = 0; i < nd->mNumMeshes; ++i) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[i]];
		printf("---%d----\n", mesh->mNumVertices);
		for (auto m = 0; m < mesh->mNumFaces; ++m) {

			const struct aiFace* face = &mesh->mFaces[m];

			for (auto k = 0; k < face->mNumIndices; ++k) {
				int index = face->mIndices[k];
				
				printf("[ %d ]%f %f %f\n", vertices++,
					mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
			}
		}
	}

	for (auto n = 0; n < nd->mNumChildren; ++n) {
		showMesh(sc, nd->mChildren[n]);
	}
}

void LoadBorn(const struct aiScene* sc, const struct aiNode* nd)
{
	for (auto i = 0; i < nd->mNumMeshes; ++i) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[i]];

		for (auto m = 0; m < mesh->mNumBones; ++m) {
			const struct aiBone* bone = mesh->mBones[m];

			printf("%s\n", bone->mName.data);

			for (auto b = 0; b < bone->mNumWeights; ++b) {
				auto vertexWeight = bone->mWeights[b];
				//printf("%d %f\n", vertexWeight.mVertexId, vertexWeight.mWeight);
			}
		}
	}

	for (auto n = 0; n < nd->mNumChildren; ++n) {
		LoadBorn(sc, nd->mChildren[n]);
	}
}