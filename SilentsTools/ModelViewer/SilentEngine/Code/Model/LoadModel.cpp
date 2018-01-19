#include "stdafx.h"
#include "LoadModel.h"


LoadModel::LoadModel(const string& fileName)
{
	m_pScene = aiImportFile(fileName.c_str(), aiProcess_JoinIdenticalVertices |		// join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure |		// perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality |		// improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials |	// remove redundant materials
		aiProcess_GenUVCoords |					// convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_TransformUVCoords |			// pre-process UV transformations (scaling, translation ...)
		aiProcess_FindInstances |				// search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights |			// limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes |				// join small meshes, if possible;
		aiProcess_GenSmoothNormals |			// generate smooth normal vectors if not existing
		aiProcess_SplitLargeMeshes |			// split large, unrenderable meshes into sub-meshes
		aiProcess_Triangulate |					// triangulate polygons with more than 3 edges
		aiProcess_ConvertToLeftHanded |			// convert everything to D3D left handed space
		aiProcess_SortByPType);					// make 'clean' meshes which consist of a single type of primitives);

	m_meshes.resize(m_pScene->mNumMeshes);
	m_numMaterial = m_pScene->mNumMaterials;
	InitScene();
}


LoadModel::~LoadModel()
{
	m_meshes.clear();
	delete m_pScene;
}

void LoadModel::InitScene()
{
	for (UINT i = 0; i < m_meshes.size(); ++i) {
		const aiMesh* pMesh = m_pScene->mMeshes[i];
		InitMesh(i, pMesh);
	}
}

void LoadModel::InitMesh(UINT index, const aiMesh * pMesh)
{
	m_meshes[index].m_vertices.reserve(pMesh->mNumVertices);
	m_meshes[index].m_indices.reserve(pMesh->mNumFaces * 3); 
	//삼각형이므로 면을 이루는 꼭지점 3개

	for (UINT i = 0; i < pMesh->mNumVertices; ++i) {
		XMFLOAT3 pos(&pMesh->mVertices[i].x);
		XMFLOAT3 normal(&pMesh->mNormals[i].x);
		XMFLOAT2 tex;
		if (pMesh->HasTextureCoords(0)) 
			tex = XMFLOAT2(&pMesh->mTextureCoords[0][i].x);
		else 
			tex = XMFLOAT2(0.0f, 0.0f);

		const vertexData data(pos, normal, tex);
		m_meshes[index].m_vertices.push_back(data);
	}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		m_meshes[index].m_indices.push_back(face.mIndices[0]);
		m_meshes[index].m_indices.push_back(face.mIndices[1]);
		m_meshes[index].m_indices.push_back(face.mIndices[2]);
	}
}

