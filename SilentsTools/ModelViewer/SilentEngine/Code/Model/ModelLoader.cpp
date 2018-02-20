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
		in >> tmpName;  //원본 모델 파일 이름
		in >> animCount; //로드할 애니메이션 개수
		
		LoadModel* tmpModel = new LoadModel(tmpName); //모델파일 로딩

		vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //애니메이션을 적재할 벡터 생성
		animStack->reserve(animCount);

		in >> tmpName; //모델의 텍스쳐 파일 이름
		matList.emplace_back(tmpName);

		for (UINT i = 0; i < animCount; ++i) {
			in >> tmpName; //애니메이션 파일 이름
			in >> start; //애니메이션 시작 프레임
			in >> end;  //애니메이션 종료 프레임
			LoadAnimation* tmpAnim = new LoadAnimation(tmpName, start, end); //애니메이션을 로딩
			animStack->push_back(tmpAnim); //애니메이션을 벡터에 적재
		}

		//모든 로딩한 파일을 벡터에 적재 (모델파일, 텍스쳐 이름, 애니메이션 파일)
		//여기서는 텍스쳐 파일 로딩을 실시하지 않음
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
	//로딩한 모델파일을 게임 상 오브젝트에 셋
	//즉 정점버퍼, 인덱스 버퍼를 생성한다는 것
	for (auto& p : m_Objects) {
		p.first->SetMeshes(pd3dDevice, pd3dCommandList);
	}
}
