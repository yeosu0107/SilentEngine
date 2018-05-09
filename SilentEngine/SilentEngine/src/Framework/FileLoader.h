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

struct FireData
{
	XMFLOAT3				m_Position;
	UINT					m_Type;
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
			data.m_MaxX = atof(st.nextToken().c_str()); //x갯수
			data.m_MaxY = atof(st.nextToken().c_str()); //y갯수

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

class FirePositionLoader
{
private:
	vector<vector<FireData>> m_PositionList;
public:
	FirePositionLoader(string fileName) {
		ifstream in(fileName);
		string tmpName;
		vector<FireData> firedatas;
		FireData		data;

		string delim = ",";
		StringTokenizer st = StringTokenizer("");
		UINT numOffire;

		getline(in, tmpName); // .csv파일 맨앞 쓰래기값 들어가는 것을 방지 

		while (getline(in, tmpName)) {
			st = StringTokenizer(tmpName, delim);
			numOffire = atof(st.nextToken().c_str());
			firedatas = vector<FireData>();

			while (numOffire != 0) {
				getline(in, tmpName);

				st = StringTokenizer(tmpName, delim);
				data.m_Position.x = atof(st.nextToken().c_str());
				data.m_Position.y = atof(st.nextToken().c_str());
				data.m_Position.z = atof(st.nextToken().c_str());
				data.m_Type = atof(st.nextToken().c_str());

				firedatas.emplace_back(data);
				numOffire -= 1;
			}

			m_PositionList.emplace_back(firedatas);
		}
		in.close();
	};
	~FirePositionLoader() {
		m_PositionList.clear();
		vector<vector<FireData>>().swap(m_PositionList);
	}
	vector<FireData>* getPosition(UINT index) {
		return &m_PositionList[index];
	}
	UINT getNumFire(UINT index) const { return (UINT)m_PositionList[index].size(); }
};