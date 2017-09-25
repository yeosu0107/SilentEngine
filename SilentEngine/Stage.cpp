#include "stdafx.h"
#include "Stage.h"

Stage::Stage(int x, int y, int index) : stage_index(index), room_index(0) {
	MapGenerator mapGenerator;
	mapGenerator.SetMap(x, y);
	currentMap = mapGenerator.getCurrentMap();

	num_room = currentMap.getMapSize();
	m_ppRoomShaders = new RoomShader*[num_room];


}

Stage::~Stage() {
	delete[] m_ppRoomShaders;
}

RoomShader::RoomShader() :num_x(0), num_y(0), num_tile(0),
	up(nullptr), down(nullptr), left(nullptr), right(nullptr)
{
	
}

RoomShader::RoomShader(int x, int y) : num_x(x), num_y(y) {
	num_tile = x*y;
}

RoomShader::~RoomShader() {

}

void RoomShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
	*pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

void RoomShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_pcbMappedGameObjects[j].m_xmcColor = m_ppObjects[j]->getColor();
		XMStoreFloat4x4(&m_pcbMappedGameObjects[j].m_xmf4x4Transform,
			XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->getMatrix())));
	}
}

void RoomShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_nObjects = num_tile;
	m_ppObjects = new CGameObject*[m_nObjects];
	CCube* tileMesh = new CCube(pd3dDevice, pd3dCommandList, 2.5f, 2.5f, 2.5f);

	//CBullet* bullet = NULL;
	//for (int i = 0; i < m_nObjects; ++i) {
	//	bullet = new CBullet();
	//	bullet->SetPosition(0.0f, 0.0f, 0.0f);
	//	//bullet->SetObject(2.5f, 2.5f, 2.5f);
	//	bullet->SetOOBB(bulletMesh->GetBoundingBox());
	//	bullet->setDie(true);
	//	m_ppObjects[i] = bullet;
	//}


	//m_ppObjects[0]->SetMesh(0, bulletMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void RoomShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	//모든 게임 객체의 인스턴싱 데이터를 버퍼에 저장한다.
	UpdateShaderVariables(pd3dCommandList);

	//하나의 정점 데이터를 사용하여 모든 게임 객체(인스턴스)들을 렌더링한다.
	m_ppObjects[0]->Render(pd3dCommandList, pCamera, m_nObjects,
		m_d3dInstancingBufferView);
}

void RoomShader::AnimateObjects(float fTime)
{
	
}
