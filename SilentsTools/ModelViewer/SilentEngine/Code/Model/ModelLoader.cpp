#include "stdafx.h"
#include "ModelLoader.h"
#include "..\..\StringTokenizer.h"
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
	StringTokenizer st= StringTokenizer("");

	//csv ���� ��� �ε�
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string�� delim�������� �����ؼ� ť�� ����
		LoadModel* tmpModel = new LoadModel(st.nextToken()); //������ �ε�
		
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
		cout << "Load Success!" << m_Objects.size() << endl;
	}
	m_numModels = m_Objects.size();
	in.close();

	//�ؽ�Ʈ ���� ��� �ε�
	//while (!in.eof()) {
	//	in >> tmpName;  //���� �� ���� �̸�
	//	in >> animCount; //�ε��� �ִϸ��̼� ����
	//	
	//	LoadModel* tmpModel = new LoadModel(tmpName); //������ �ε�

	//	vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //�ִϸ��̼��� ������ ���� ����
	//	animStack->reserve(animCount);

	//	in >> tmpName; //���� �ؽ��� ���� �̸�
	//	matList.emplace_back(tmpName);

	//	for (UINT i = 0; i < animCount; ++i) {
	//		in >> tmpName; //�ִϸ��̼� ���� �̸�
	//		LoadAnimation* tmpAnim = new LoadAnimation(tmpName); //�ִϸ��̼��� �ε�
	//		animStack->push_back(tmpAnim); //�ִϸ��̼��� ���Ϳ� ����
	//	}

	//	//��� �ε��� ������ ���Ϳ� ���� (������, �ؽ��� �̸�, �ִϸ��̼� ����)
	//	//���⼭�� �ؽ��� ���� �ε��� �ǽ����� ����
	//	m_Objects.emplace_back(make_pair(tmpModel, animStack));
	//	cout << "Load Success!" << m_Objects.size() << endl;
	//}

	
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
