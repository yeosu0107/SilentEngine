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
	vector<XMFLOAT3>				m_Position;
	vector<UINT>							m_FireType;
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
			cout << "Effect Data Load Success " << m_textureList.size() << endl;
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
	vector<FireData> m_PositionList;
public:
	FirePositionLoader(string fileName) {
		ifstream in(fileName);
		string tmpName;
		vector<XMFLOAT3> firedatas;
		vector<UINT>		fireType;

		XMFLOAT3 posData;
		UINT typeData;

		string delim = ",";
		StringTokenizer st = StringTokenizer("");
		UINT numOffire;
		int nRoomNo = 0;

		m_PositionList.resize(MAX_MAP);

		getline(in, tmpName); // .csv파일 맨앞 쓰래기값 들어가는 것을 방지 
		firedatas.resize(10);
		fireType.resize(10);

		while (getline(in, tmpName)) {
			st = StringTokenizer(tmpName, delim);
			numOffire = atof(st.nextToken().c_str());

			for (int i = 10; i > numOffire; --i) {
				posData.x = 10000.0f;
				posData.y = 10000.0f;
				posData.z = 10000.0f;

				fireType[i - 1] = 0;
				firedatas[i - 1] = posData;
			}
			while (numOffire != 0) {
				getline(in, tmpName);

				st = StringTokenizer(tmpName, delim);
				posData.x = atof(st.nextToken().c_str());
				posData.y = atof(st.nextToken().c_str());
				posData.z = atof(st.nextToken().c_str());
				typeData = atof(st.nextToken().c_str());

				numOffire -= 1;
				firedatas[numOffire] = posData;
				fireType[numOffire] = typeData;
			}

			m_PositionList[nRoomNo].m_Position = firedatas;
			m_PositionList[nRoomNo].m_FireType = fireType;
			nRoomNo++;
		}
		in.close();
		cout << "Extra Data Load Success" << endl;
	};
	~FirePositionLoader() {
		m_PositionList.clear();
		vector<FireData>().swap(m_PositionList);
	}
	XMFLOAT3* getPosition(UINT index) {
		return m_PositionList[index].m_Position.data();
	}
	UINT getMaxFire(UINT index) const {
		return m_PositionList[index].m_Position.size();
	}
	UINT* getType(UINT index) {
		return m_PositionList[index].m_FireType.data();
	}
};