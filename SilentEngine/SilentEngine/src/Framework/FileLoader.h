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
			tmpName = st.nextToken(); //ÅØ½ºÃÄ
			data.m_texture = wstring(tmpName.begin(), tmpName.end());
			tmpName = st.nextToken(); //³ë¸»¸Ê
			data.m_normal = wstring(tmpName.begin(), tmpName.end());
			data.m_MaxX = atof(st.nextToken().c_str()); //x°¹¼ö
			data.m_MaxY = atof(st.nextToken().c_str()); //y°¹¼ö

			m_textureList.emplace_back(data);
		}
		in.close();
	};
	~EffectLoader() {
		m_textureList.clear();
		vector<TextureDataForm>().swap(m_textureList);
	}
	TextureDataForm* getTextureFile(UINT index) {
		return &m_textureList[index];
	}
	UINT getNumTexture() const { return (UINT)m_textureList.size(); }
};