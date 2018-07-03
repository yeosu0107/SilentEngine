#pragma once

#include "LoadModel.h"
#include "Animation.h"
class ModelLoader
{
protected:
	UINT m_numModels;
	vector<pair<LoadModel*, vector<LoadAnimation*>*>> m_Objects;
	vector<string>		matList;
	vector<string>		normMatList;
public:
	ModelLoader();
	ModelLoader(string fileName);
	~ModelLoader();

	void LodingModels(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	LoadModel* getModel(UINT index) { return m_Objects[index].first; }
	UINT getAnimCount(UINT index) { return (UINT)m_Objects[index].second->size(); }
	LoadAnimation** getAnim(UINT index) { return m_Objects[index].second->data(); }
	wstring getMat(UINT index) { 
		return wstring(matList[index].begin(), matList[index].end());
	}
	wstring getNorMat(UINT index) {
		return wstring(normMatList[index].begin(), normMatList[index].end());
	}
	bool isMat(UINT index) {
		if (matList[index] != "null")
			return true;
		return false;
	}
	bool isNormat(UINT index) {
		if (normMatList[index] != "null")
			return true;
		return false;
	}

	UINT getNumModel() const { return m_numModels; }
};



class MapLoader : public ModelLoader
{
private:
	struct StartList {
		XMFLOAT3	point[4];
		bool		bFogEnable;
		XMFLOAT3* returnPoint() { return point; }
	};
	vector<StartList>	m_startPoint;
	
public:
	MapLoader(string fileName);
	~MapLoader();

	StartList getStartpoint(UINT index) const { return m_startPoint[index]; }
	bool getFogEnabled(UINT index) const { return m_startPoint[index].bFogEnable; }
};