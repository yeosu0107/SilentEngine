#include "stdafx.h"
#include "ModelLoader.h"
#include "StringTokenizer.h"
#include <fstream>


ModelLoader::ModelLoader() :
	m_numModels(0)
{
}

ModelLoader::ModelLoader(string fileName)
{
	ifstream in(fileName);

	string tmpName;
	string delim = ",";
	UINT animCount;
	UINT index;
	StringTokenizer st = StringTokenizer("");

	//csv ���� ��� �ε�
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string�� delim�������� �����ؼ� ť�� ����
		LoadModel* tmpModel = new LoadModel(st.nextToken(), false); //������ �ε�

		animCount = atoi(st.nextToken().c_str()); //string�� int������ ��ȯ
		vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //�ִϸ��̼��� ������ ���� ����
		animStack->reserve(animCount);

		matList.emplace_back(st.nextToken()); //�ؽ��� ����

		for (UINT i = 0; i < animCount; ++i) {
			getline(in, tmpName);
			st = StringTokenizer(tmpName, delim);
			LoadAnimation* tmpAnim = new LoadAnimation(st.nextToken()); //�ִϸ��̼��� �ε�

			if (st.countTokens() != 0) {
				//���� �ִϸ��̼��� ������ �ƴ϶�� ���� �ε��� ������ ť�� ����Ǿ�����
				index = atoi(st.nextToken().c_str()); //���� �ִϸ��̼� �ε���
				tmpAnim->DisableLoof(index);
				//�ִϸ��̼� ������ �����ϰ�, ���� �ִϸ��̼� �ε����� ����
			}
			animStack->push_back(tmpAnim); //�ִϸ��̼��� ���Ϳ� ����
		}
		m_Objects.emplace_back(make_pair(tmpModel, animStack));
		cout << "Model Load Success!" << m_Objects.size() << endl;
	}
	m_numModels = (UINT)m_Objects.size();
	in.close();
}

ModelLoader::~ModelLoader()
{

}

void ModelLoader::LodingModels(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	//�ε��� �������� ���� �� ������Ʈ�� ��
	//�� ��������, �ε��� ���۸� �����Ѵٴ� ��
	for (auto& p : m_Objects) {
		p.first->SetMeshes(pd3dDevice, pd3dCommandList);
	}
}

MapLoader::MapLoader(string fileName)
{
	ifstream in(fileName);

	string tmpName;
	string delim = ",";

	float horizontal, vertical, height;
	StringTokenizer st = StringTokenizer("");

	//csv ���� ��� �ε�
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string�� delim�������� �����ؼ� ť�� ����
		LoadModel* tmpModel = new LoadModel(st.nextToken(), true); //������ �ε�
		matList.emplace_back(st.nextToken()); //�ؽ��� ����

		horizontal = atof(st.nextToken().c_str());
		height = atof(st.nextToken().c_str());
		vertical = atof(st.nextToken().c_str());
		StartList point = {
			Point(horizontal, height, 0), Point(-horizontal, height, 0),		//WEST, EAST
			Point(0,height, -vertical), Point(0,height, vertical)				//SOUTH, NORTH
		};
		
		m_Objects.emplace_back(make_pair(tmpModel, nullptr)); //map ������ �ִϸ��̼��� ����.
		m_startPoint.push_back(point);

		cout << "Map Load Success!" << m_Objects.size() << endl;
	}
	m_numModels = (UINT)m_Objects.size();
	in.close();
}


MapLoader::~MapLoader()
{
}
