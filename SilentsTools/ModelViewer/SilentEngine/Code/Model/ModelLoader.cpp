#include "stdafx.h"
#include "ModelLoader.h"

#include <fstream>

ModelLoader::ModelLoader() :
	m_numModels(0)
{
}

ModelLoader::ModelLoader(string fileName)
{
	ifstream in(fileName);

	string tmpName;
	wstring matName;
	UINT animCount;

	while (!in.eof()) {
		in >> tmpName;
		in >> animCount;
		
		LoadModel* tmpModel = new LoadModel(tmpName);

		vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>;
		animStack->reserve(animCount);

		in >> tmpName;
		matList.emplace_back(tmpName);

		for (UINT i = 0; i < animCount; ++i) {
			in >> tmpName;
			LoadAnimation* tmpAnim = new LoadAnimation(tmpName);
			animStack->push_back(tmpAnim);
		}

		m_Objects.emplace_back(make_pair(tmpModel, animStack));
	}

	m_numModels = m_Objects.size();
	in.close();
}

ModelLoader::~ModelLoader()
{

}

void ModelLoader::LodingModels(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	for (auto& p : m_Objects) {
		p.first->SetMeshes(pd3dDevice, pd3dCommandList);
	}
}
