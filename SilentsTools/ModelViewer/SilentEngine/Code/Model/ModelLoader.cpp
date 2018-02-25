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

	//csv 파일 기반 로드
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string을 delim기준으로 분할해서 큐에 저장
		LoadModel* tmpModel = new LoadModel(st.nextToken()); //모델파일 로딩
		
		animCount = atoi(st.nextToken().c_str()); //string을 int형으로 변환
		vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //애니메이션을 적재할 벡터 생성
		animStack->reserve(animCount);
		
		matList.emplace_back(st.nextToken()); //텍스쳐 파일

		for (UINT i = 0; i < animCount; ++i) {
			getline(in, tmpName);
			st = StringTokenizer(tmpName, delim);
			LoadAnimation* tmpAnim = new LoadAnimation(st.nextToken()); //애니메이션을 로딩
			
			if (st.countTokens() != 0) {
				//만약 애니메이션이 루프가 아니라면 다음 인덱스 파일이 큐에 저장되어있음
				index = atoi(st.nextToken().c_str()); //다음 애니메이션 인덱스
				tmpAnim->DisableLoof(index);
				//애니메이션 루프를 해제하고, 다음 애니메이션 인덱스를 설정
			}
			animStack->push_back(tmpAnim); //애니메이션을 벡터에 적재
		}
		m_Objects.emplace_back(make_pair(tmpModel, animStack));
		cout << "Load Success!" << m_Objects.size() << endl;
	}
	m_numModels = m_Objects.size();
	in.close();

	//텍스트 파일 기반 로드
	//while (!in.eof()) {
	//	in >> tmpName;  //원본 모델 파일 이름
	//	in >> animCount; //로드할 애니메이션 개수
	//	
	//	LoadModel* tmpModel = new LoadModel(tmpName); //모델파일 로딩

	//	vector<LoadAnimation*>* animStack = new vector<LoadAnimation*>; //애니메이션을 적재할 벡터 생성
	//	animStack->reserve(animCount);

	//	in >> tmpName; //모델의 텍스쳐 파일 이름
	//	matList.emplace_back(tmpName);

	//	for (UINT i = 0; i < animCount; ++i) {
	//		in >> tmpName; //애니메이션 파일 이름
	//		LoadAnimation* tmpAnim = new LoadAnimation(tmpName); //애니메이션을 로딩
	//		animStack->push_back(tmpAnim); //애니메이션을 벡터에 적재
	//	}

	//	//모든 로딩한 파일을 벡터에 적재 (모델파일, 텍스쳐 이름, 애니메이션 파일)
	//	//여기서는 텍스쳐 파일 로딩을 실시하지 않음
	//	m_Objects.emplace_back(make_pair(tmpModel, animStack));
	//	cout << "Load Success!" << m_Objects.size() << endl;
	//}

	
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
