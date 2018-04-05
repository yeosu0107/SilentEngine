#include "stdafx.h"
#include "Scene.h"
#include "..\Shaders\PlayerShader.h"
#include "..\Shaders\PaticleShader.h"
#include "..\Model\InstanceModelShader.h"

ostream& operator<<(ostream& os, XMFLOAT3& p)
{
	os << "[" << p.x << ", " << p.y << ", " << p.z << "]" << endl;
	return os;
}

Scene::Scene() : m_physics(nullptr)
{
}

Scene::~Scene()
{
}

void Scene::BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
	BuildRootSignature(pDevice, pCommandList);
}

void Scene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	
}

TestScene::TestScene()
{
	m_physics = new BasePhysX(60.0f);
	m_testPlayer = nullptr;
	m_nowRoom = START_ROOM;
	m_isRoomChange = Door(0, START_SOUTH, true);
}

TestScene::~TestScene()
{
	delete m_physics;
}

void TestScene::BuildRootSignature(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2; //Game Objects
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //Texture2DArray
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Game Objects
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Materials
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 5; //Lights
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //Texture2DArray
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	ThrowIfFailed(pDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void TestScene::CreateShaderVariables(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pd3dcbLights = std::make_unique<UploadBuffer<LIGHTS>>(pDevice, 1, true);
	m_pd3dcbMaterials = std::make_unique<UploadBuffer<MATERIALS>>(pDevice, 1, true);
}

void TestScene::UpdateShaderVarialbes() {
	m_pd3dcbLights->CopyData(0, *m_pLights);
	m_pd3dcbMaterials->CopyData(0, *m_pMaterials);
}

void TestScene::Update(const Timer & gt)
{
	m_physics->stepPhysics(false);

	for (UINT i = 0; i < m_nShaders; ++i) {
		m_ppShaders[i]->Animate(gt.DeltaTime());
	}

	if (m_Projectile) {
		m_testTimer += 1;
		if (m_testTimer % 50 == 0) {
			XMFLOAT3 tPos = m_Enemys[0]->GetPosition();
			tPos.y += 20.0f;
			XMFLOAT3 ttPos = m_testPlayer->GetPosition();
			ttPos.y += 20.0f;
			
			m_Projectile->Shoot(m_physics, tPos, ttPos);
			m_testTimer = 0;
		}
		XMFLOAT3* pos;
		UINT tmp=0;
		pos = m_Projectile->returnCollisionPos(tmp);
		if(tmp>0) 
			m_EffectShaders->SetPos(pos, tmp);
	}
	m_Room[m_nowRoom]->Animate(gt.DeltaTime(), m_testPlayer->GetPosition(), m_isRoomChange);
	m_EffectShaders->Animate(gt.DeltaTime());

	RoomChange();
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{

	BuildRootSignature(pDevice, pCommandList);

	BuildLightsAndMaterials();
	CreateShaderVariables(pDevice, pCommandList);

	m_Camera = make_unique<CThirdPersonCamera>();
	m_Camera->InitCamera(pDevice, pCommandList);
	m_Camera->SetOffset(XMFLOAT3(0.0f, 60.0f, -100.0f));
	m_Camera->SetTimeLag(0.30f);

	m_nShaders = 1;
	m_ppShaders = new Shaders*[m_nShaders];

	m_nRoom = 2;
	m_Room = new Room*[m_nRoom];
	m_Room[0] = new Room(Room::RoomType::tree);
	m_Room[1] = new Room(Room::RoomType::brick);
	
	PlayerShader* player = new PlayerShader(1, m_Camera.get());
	player->SetLightsUploadBuffer(m_pd3dcbLights.get());
	player->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	
	m_ppShaders[0] = player;
	
	PaticleShader<PaticleObject>* Explosion = new PaticleShader<PaticleObject>();
	Explosion->SetLightsUploadBuffer(m_pd3dcbLights.get());
	Explosion->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	Explosion->SetCamera(m_Camera.get());
	Explosion->BuildObjects(pDevice, pCommandList, 2, globalEffects->getTextureFile(1));
	
	m_EffectShaders = Explosion;

	InstanceModelShader* map = nullptr;



	for (UINT i = 0; i < m_nRoom; ++i) {
		map= new InstanceModelShader(i+8);
		map->SetLightsUploadBuffer(m_pd3dcbLights.get());
		map->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
		map->BuildObjects(pDevice, pCommandList, 2);
		m_Room[i]->SetMapShader(map);
		m_Room[i]->SetStartPoint(globalMaps->getStartpoint(i+8).returnPoint());
	}

	EnemyShader<Enemy>* eShader = new EnemyShader<Enemy>(0);
	eShader->SetLightsUploadBuffer(m_pd3dcbLights.get());
	eShader->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	eShader->BuildObjects(pDevice, pCommandList,2, m_physics);

	ProjectileShader* bullet = new ProjectileShader();
	bullet->SetLightsUploadBuffer(m_pd3dcbLights.get());
	bullet->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	bullet->SetCamera(m_Camera.get());
	bullet->BuildObjects(pDevice, pCommandList ,2, globalEffects->getTextureFile(0));

	for(UINT i=0; i<m_nShaders; ++i)
		m_ppShaders[i]->BuildObjects(pDevice, pCommandList,2, m_physics);

	m_testPlayer = player->getPlayer(0);
	
	m_Room[0]->SetEnemyShader(eShader);
	m_Room[0]->SetProjectileShader(bullet);

	RoomChange();

}

void TestScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	UpdateShaderVarialbes();

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(2, d3dcbMaterialsGpuVirtualAddress); //Materials
	//플레이어 랜더링
	for (UINT i = 0; i<m_nShaders; ++i)
		m_ppShaders[i]->Render(pCommandList, m_Camera.get());
	//맵, 적, 발사체 랜더링
	m_Room[m_nowRoom]->Render(pCommandList, m_Camera.get());
	//이펙트 파티클 랜더링
	m_EffectShaders->Render(pCommandList, m_Camera.get());
}

bool TestScene::OnKeyboardInput(const Timer& gt, UCHAR *pKeysBuffer)
{
	DWORD moveInpout = 0;
	DWORD input = 0;

	if (GetAsyncKeyState('W') & 0x8000)
		moveInpout |= DIR_FORWARD;

	if (GetAsyncKeyState('S') & 0x8000)
		moveInpout |= DIR_BACKWARD;

	if (GetAsyncKeyState('A') & 0x8000)
		moveInpout |= DIR_LEFT;

	if (GetAsyncKeyState('D') & 0x8000)
		moveInpout |= DIR_RIGHT;

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		cout << m_testPlayer->GetPosition();
	
	//if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	//	RoomChange(1, START_WEST);

	if(!m_testPlayer->Movement(input))
		m_testPlayer->Move(moveInpout, 1.1f);

	return false;
}

bool TestScene::OnMouseDown(HWND& hWin, WPARAM btnState, int x, int y)
{
	return false;
}

bool TestScene::OnMouseUp(HWND& hWin, WPARAM btnState, int x, int y)
{
	return false;
}

bool TestScene::OnMouseMove(HWND& hWin, WPARAM btnState, float x, float y)
{
	m_testPlayer->Rotate(y, x, 0.0f);
	return true;
}

void TestScene::RoomChange()
{
	if (m_nowRoom == m_isRoomChange.m_roomNum)
		return;

	if (!m_isRoomChange.m_isChange)
		return;
	
	if (m_nowRoom != START_ROOM)
		m_Room[m_nowRoom]->RegistShader(m_physics, false, START_NON);

	Point* playerPos;
	playerPos = m_Room[m_isRoomChange.m_roomNum]->RegistShader(m_physics, true, m_isRoomChange.m_dir);

	m_testPlayer->SetPosition(playerPos->xPos, playerPos->yPos, playerPos->zPos);
	//카메라 맵 이동 함수 제작해야 됨
	if (m_Room[m_isRoomChange.m_roomNum]->IsEnemy())
		m_Enemys = m_Room[m_isRoomChange.m_roomNum]->GetEnemyShader()->getObjects(m_nEnemy);
	else
		m_Enemys = nullptr;

	if (m_Room[m_isRoomChange.m_roomNum]->IsProjectile())
		m_Projectile = m_Room[m_isRoomChange.m_roomNum]->GetProjectileShader();
	else
		m_Projectile = nullptr;
	
	m_nowRoom = m_isRoomChange.m_roomNum;
	m_isRoomChange.m_isChange = false;
}

void TestScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS();
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 2000.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.25f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(0.0f, 125.0, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = true;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 60.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 1.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(-150.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 8.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pMaterials = new MATERIALS();
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

/*
XMFLOAT4				m_xmf4Ambient;	// 앰비언트 반사 색상
XMFLOAT4				m_xmf4Diffuse;
XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
XMFLOAT4				m_xmf4Emissive;
*/
