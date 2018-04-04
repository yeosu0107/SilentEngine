#pragma once
#include "stdafx.h"
#include "..\Model\StringTokenizer.h"

struct TextureDataForm
{
	wstring						m_texture;
	wstring						m_normal;
	float							m_MaxX;
	float							m_MaxY;
};

class EffectLoader
{
private:
	vector<TextureDataForm> m_textureList;
public:
	EffectLoader(string fileName) {
		ifstream in(fileName);
		string tmpName;
		TextureDataForm		data;
		
		string delim = ",";
		StringTokenizer st = StringTokenizer("");

		while (getline(in, tmpName)) {
			st = StringTokenizer(tmpName, delim);
			tmpName = st.nextToken(); //텍스쳐
			data.m_texture = wstring(tmpName.begin(), tmpName.end());
			tmpName = st.nextToken(); //노말맵
			data.m_normal = wstring(tmpName.begin(), tmpName.end());
			data.m_MaxX = atoi(st.nextToken().c_str());
			data.m_MaxY = atoi(st.nextToken().c_str());

			m_textureList.emplace_back(data);
		}
	};
	~EffectLoader() {
		m_textureList.clear();
		vector<TextureDataForm>().swap(m_textureList);
	}
	TextureDataForm* getTextureFile(UINT index) {
		return &m_textureList[index];
	}
	UINT getNumTexture() const { return m_textureList.size(); }
};