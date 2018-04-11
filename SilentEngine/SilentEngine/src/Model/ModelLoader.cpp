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

	//csv 파일 기반 로드
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string을 delim기준으로 분할해서 큐에 저장
		LoadModel* tmpModel = new LoadModel(st.nextToken(), false); //모델파일 로딩

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
	//로딩한 모델파일을 게임 상 오브젝트에 셋
	//즉 정점버퍼, 인덱스 버퍼를 생성한다는 것
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

	//csv 파일 기반 로드
	while (getline(in, tmpName)) {
		st = StringTokenizer(tmpName, delim); //string을 delim기준으로 분할해서 큐에 저장
		LoadModel* tmpModel = new LoadModel(st.nextToken(), true); //모델파일 로딩
		matList.emplace_back(st.nextToken()); //텍스쳐 파일

		horizontal = atof(st.nextToken().c_str());
		height = atof(st.nextToken().c_str());
		vertical = atof(st.nextToken().c_str());
		StartList point = {
			Point(horizontal, height, 0), Point(-horizontal, height, 0),		//WEST, EAST
			Point(0,height, -vertical), Point(0,height, vertical)				//SOUTH, NORTH
		};
		
		m_Objects.emplace_back(make_pair(tmpModel, nullptr)); //map 파일은 애니메이션이 없다.
		m_startPoint.push_back(point);

		cout << "Map Load Success!" << m_Objects.size() << endl;
	}
	m_numModels = (UINT)m_Objects.size();
	in.close();
}


MapLoader::~MapLoader()
{
}
