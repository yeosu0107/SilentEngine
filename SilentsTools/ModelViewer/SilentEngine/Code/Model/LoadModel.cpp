#include "stdafx.h"
#include "LoadModel.h"

ModelMesh::ModelMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	mesh& meshData)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = (int)meshData.m_vertices.size();
	m_nIndices = (int)meshData.m_indices.size();

	m_nStride = sizeof(vertexDatas);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	//버텍스 버퍼
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, meshData.m_vertices.data(), m_nStride * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	//인덱스 버퍼
	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, meshData.m_indices.data(), sizeof(int)*m_nIndices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT)*m_nIndices;
}

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
	m_numBones = 0;


	InitScene();
	for (const auto& p : m_Bones) {
		cout << p.first << endl;
	}
	m_ModelMeshes.resize(m_meshes.size());
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
		InitBones(i, pMesh);
		
		m_numVertices += (UINT)m_meshes[i].m_vertices.size();
	}
	m_numBones = (UINT)m_Bones.size();

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

		const vertexDatas data(pos, normal, tex);
		m_meshes[index].m_vertices.push_back(data);
	}

	for (UINT i = 0; i < pMesh->mNumFaces; ++i) {
		const aiFace& face = pMesh->mFaces[i];
		m_meshes[index].m_indices.push_back(face.mIndices[0]);
		m_meshes[index].m_indices.push_back(face.mIndices[1]);
		m_meshes[index].m_indices.push_back(face.mIndices[2]);
	}
}

void LoadModel::SetMeshes(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (UINT i = 0; i < m_ModelMeshes.size(); ++i) {
		ModelMesh* tmp = new ModelMesh(pd3dDevice, pd3dCommandList, m_meshes[i]);
		m_ModelMeshes[i] = tmp;
	}
}

void LoadModel::InitBones(UINT index, const aiMesh* pMesh)
{
	for (UINT i = 0; i < pMesh->mNumBones; ++i) {
		int BoneIndex = -1;
		string BoneName(pMesh->mBones[i]->mName.data);

		int tmpIndex = 0;
		for (const auto& p : m_Bones) { //이미 존재하는 뼈인지 검색
			if (p.first == BoneName) {
				BoneIndex = tmpIndex;
				//현재 뼈가 이미 벡터에 저장된 뼈일 경우
				//인덱스를 해당 뼈의 인덱스로 저장
				break;
			}
			tmpIndex++;
		}

		if (BoneIndex < 0) { //없으면 새로 추가
			BoneIndex = (int)m_Bones.size();
			//새로 저장하는 뼈일 경우 
			//인덱스는 현재 뼈의 개수 (0개일 경우 0부터 시작)

			Bone bone;
			bone.BoneOffset = XMMATRIX(&pMesh->mBones[BoneIndex]->mOffsetMatrix.a1);
			m_Bones.emplace_back(make_pair(BoneName, bone));
		}

		const aiBone* pBone = pMesh->mBones[BoneIndex];
		for (UINT b = 0; b < pBone->mNumWeights; ++b) {
			UINT vertexID = pBone->mWeights[b].mVertexId;
			float weight = pBone->mWeights[b].mWeight;
			m_meshes[index].m_vertices[vertexID].AddBoneData(BoneIndex, weight);
		}
	}
}