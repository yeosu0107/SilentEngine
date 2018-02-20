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
	UINT animCount;
	float start, end;

	while (!in.eof()) {
		in >> tmpName;  //���� �� ���� �̸�
		in >> animCount; //�ε��� �ִϸ��̼� ����
		
		LoadModel* tmpModel = new LoadModel(tmpName); //������ �ε�

		vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //�ִϸ��̼��� ������ ���� ����
		animStack->reserve(animCount);

		in >> tmpName; //���� �ؽ��� ���� �̸�
		matList.emplace_back(tmpName);

		for (UINT i = 0; i < animCount; ++i) {
			in >> tmpName; //�ִϸ��̼� ���� �̸�
			in >> start; //�ִϸ��̼� ���� ������
			in >> end;  //�ִϸ��̼� ���� ������
			LoadAnimation* tmpAnim = new LoadAnimation(tmpName, start, end); //�ִϸ��̼��� �ε�
			animStack->push_back(tmpAnim); //�ִϸ��̼��� ���Ϳ� ����
		}

		//��� �ε��� ������ ���Ϳ� ���� (������, �ؽ��� �̸�, �ִϸ��̼� ����)
		//���⼭�� �ؽ��� ���� �ε��� �ǽ����� ����
		m_Objects.emplace_back(make_pair(tmpModel, animStack));
		cout << "Load Success!" << m_Objects.size() << endl;
	}

	m_numModels = m_Objects.size();
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
