#pragma once

#include "LoadModel.h"
#include "Animation.h"

class ModelLoader
{
private:
	UINT m_numModels;
	vector<pair<LoadModel*, vector<LoadAnimation*>*>> m_Objects;
	vector<string> matList;
public:
	ModelLoader();
	ModelLoader(string fileName);
	~ModelLoader();

	void LodingModels(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	LoadModel* getModel(UINT index) { return m_Objects[index].first; }
	UINT getAnimCount(UINT index) { return m_Objects[index].second->size(); }
	LoadAnimation** getAnim(UINT index) { return m_Objects[index].second->data(); }
	wstring getMat(UINT index) { 
		return wstring(matList[index].begin(), matList[index].end());
	}
	bool isMat(UINT index) {
		if (matList[index] != "null")
			return true;
		return false;
	}

	UINT getNumModel() const { return m_numModels; }
};