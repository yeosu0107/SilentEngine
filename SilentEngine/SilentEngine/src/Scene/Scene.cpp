#include "stdafx.h"
#include "Scene.h"
#include "..\Room\Stage.h"
#include "..\Shaders\PaticleShader.h"
#include "..\Enemys\GhostEnemy.h"
#include "..\Enemys\CreepArm.h"
#include "..\Enemys\skull.h"
#include "..\Enemys\Rich.h"
#include <ctime>

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

	RoomSetting();
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

	D3D12_ROOT_PARAMETER pd3dRootParameters[6];

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

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[5].Descriptor.ShaderRegister = 8; //Fog
	pd3dRootParameters[5].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
	m_pd3dcbMaterials = std::make_unique<UploadBuffer<MATERIALS>>(pDevice, 1, true);
	m_pd3dcbFog = std::make_unique<UploadBuffer<CB_FOG_INFO>>(pDevice, 1, true);
}

void TestScene::UpdateShaderVarialbes() {
	m_pLights->UpdateShaderVariables();
	m_pd3dcbMaterials->CopyData(0, *m_pMaterials);
	m_pd3dcbFog->CopyData(0, *m_pFog);
}

void TestScene::Update(const Timer & gt)
{
	//물리 시물레이트
	m_physics->stepPhysics(false);
	//플레이어 애니메이트
	m_playerShader->Animate(gt.DeltaTime());
	//게이트 애니메이트(문 열리는 애니메이션, 방 클리어 시만 수행)
	if (m_Room[m_nowRoom]->IsClear())
		m_gateShader->Animate(gt.DeltaTime(), m_Room[m_nowRoom]->getNextRoom());

	//발사체 있을 경우 발사체 경로계산 및 발사
	if (m_Projectile) {
		XMFLOAT3* pos;
		UINT collisionCount=0;
		pos = m_Projectile->returnCollisionPos(collisionCount);
		if(collisionCount>0)
			m_EffectShaders->SetPos(pos, collisionCount);
	}
	//적 및 발사체 애니메이트 
	m_Room[m_nowRoom]->Animate(gt.DeltaTime(), m_testPlayer->GetPosition(), m_isRoomChange);
	//m_isRoomChange변수에 방 이동정보가 들어옴 (방 이동을 하는지, 어느 방으로 이동하는지)


	//빌보드 이펙트 애니메이트
	m_EffectShaders->Animate(gt.DeltaTime());
	m_hitEffectShaders->Animate(gt.DeltaTime());

	m_pFadeEffectShader->Animate(gt.DeltaTime());

	for (int i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Animate(gt.DeltaTime());
	
	RoomChange();	//방 전환 (true일 경우만 작동)
	RoomFade();		//방 전환이 있을 경우 페이드IN/OUT 처리
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pLights = new LightManagement();
	m_pLights->BuildObject(pDevice, pCommandList, 45.0f, XMFLOAT3(1.0f, 0.0f, .0f));
	
	EffectLoader* globalEffects = GlobalVal::getInstance()->getEffectLoader();
	MapLoader* globalMaps = GlobalVal::getInstance()->getMapLoader();
	FirePositionLoader* loader = GlobalVal::getInstance()->getFirePos();

	const UINT KindOfEnemy = 5;

	ModelShader** enemyShader = new ModelShader*[KindOfEnemy];
	int enemyNum[KindOfEnemy] = { 6,2,4,3,1 };
	
	BuildRootSignature(pDevice, pCommandList);
	BuildLightsAndMaterials();
	BuildFog();
	CreateShaderVariables(pDevice, pCommandList);

	m_Camera = make_unique<CThirdPersonCamera>();
	m_Camera->InitCamera(pDevice, pCommandList);
	m_Camera->SetOffset(XMFLOAT3(0.0f, 100.0f, -60.0f));
	m_Camera->SetTimeLag(0.30f);
	
	PlayerShader* player = new PlayerShader(1, m_Camera.get());
	player->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	player->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());

	player->BuildObjects(pDevice, pCommandList, 2, m_physics);
	m_playerShader = player;
	
	PaticleShader<PaticleObject>* Explosion = new PaticleShader<PaticleObject>();
	Explosion->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	Explosion->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	Explosion->SetFogUploadBuffer(m_pd3dcbFog.get());
	Explosion->SetCamera(m_Camera.get());
	Explosion->BuildObjects(pDevice, pCommandList, 2, globalEffects->getTextureFile(1));
	m_EffectShaders = Explosion;

	PaticleShader<HitPaticle>* hitShader = new PaticleShader<HitPaticle>();
	hitShader->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	hitShader->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	hitShader->SetFogUploadBuffer(m_pd3dcbFog.get());
	hitShader->SetCamera(m_Camera.get());
	hitShader->BuildObjects(pDevice, pCommandList, 2, globalEffects->getTextureFile(3));
	hitShader->setAnimSpeed(50.0f);
	m_hitEffectShaders = hitShader;

	PaticleShader<PaticleObject>* fireObjectShaders = new PaticleShader<PaticleObject>();
	fireObjectShaders->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	fireObjectShaders->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	fireObjectShaders->SetFogUploadBuffer(m_pd3dcbFog.get());
	fireObjectShaders->SetCamera(m_Camera.get());
	fireObjectShaders->BuildObjects(pDevice, pCommandList, 2, globalEffects->getTextureFile(2));
	fireObjectShaders->setLoop(true);
	fireObjectShaders->setAnimSpeed(50.0f);
	fireObjectShaders->SetRotateLockXZ(true);
	
	InstanceModelShader** map = new InstanceModelShader*[MAX_MAP];
	for (UINT i = 0; i < MAX_MAP; ++i) {
		map[i] = new MapShader(i);
		map[i]->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
		map[i]->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
		map[i]->SetFogUploadBuffer(m_pd3dcbFog.get());
		map[i]->BuildObjects(pDevice, pCommandList, 2);
	}
	for (UINT i = 0; i < m_nRoom; ++i) {
		m_Room[i]->SetMapShader(map[m_Room[i]->getType()]);
		m_Room[i]->SetFireShader(fireObjectShaders);
		m_Room[i]->SetFirePosition(loader->getPosition(m_Room[i]->getType()));
		m_Room[i]->SetStartPoint(globalMaps->getStartpoint(m_Room[i]->getType()).returnPoint());
	}


	InstanceModelShader* gateShader = new InstanceModelShader(10);
	gateShader->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	gateShader->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	gateShader->SetFogUploadBuffer(m_pd3dcbFog.get());
	gateShader->BuildObjects(pDevice, pCommandList, 2);
	gateShader->setPhys(m_physics);
	m_gateShader = gateShader;

	enemyShader[0] = new EnemyShader<CreepArm>(4);
	enemyShader[1] = new EnemyShader<Enemy>(3);
	enemyShader[2] = new EnemyShader<Ghost>(2);
	enemyShader[3] = new EnemyShader<Skull>(5);
	enemyShader[4] = new EnemyShader<Rich>(6);

	for (int i = 0; i < 5; ++i) {
		enemyShader[i]->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
		enemyShader[i]->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
		enemyShader[i]->SetFogUploadBuffer(m_pd3dcbFog.get());
		enemyShader[i]->BuildObjects(pDevice, pCommandList, 2, (int*)enemyNum[i]);
	}

	ProjectileShader* bullet = new ProjectileShader();
	bullet->SetLightsUploadBuffer(m_pLights->LightUploadBuffer());
	bullet->SetMaterialUploadBuffer(m_pd3dcbMaterials.get());
	bullet->SetFogUploadBuffer(m_pd3dcbFog.get());
	bullet->SetCamera(m_Camera.get());
	bullet->BuildObjects(pDevice, pCommandList ,2, globalEffects->getTextureFile(0));
	bullet->setPhys(m_physics);

	m_pFadeEffectShader = new FadeEffectShader();
	m_pFadeEffectShader->BuildObjects(pDevice, pCommandList, 1);

	m_testPlayer = player->getPlayer(0);
	m_testPlayer->GetPosition();
	GlobalVal::getInstance()->setPlayer(m_testPlayer);
	
	GlobalVal::getInstance()->setHitPaticle(m_hitEffectShaders);

	for (int i = 0; i < m_nRoom; ++i) {
		UINT index = rand() % KindOfEnemy;
		m_Room[i]->SetEnemyShader(enemyShader[index]);
		if (index >= 2)
			m_Room[i]->SetProjectileShader(bullet);
	}

	RoomChange();

	m_nUIShaders = 2;
	m_ppUIShaders.resize(m_nUIShaders);

	UIHPBarShaders* pHPBar = new UIHPBarShaders();
	pHPBar->BuildObjects(pDevice, pCommandList, 1, m_testPlayer);
	m_ppUIShaders[0] = pHPBar;

	UIMiniMapShaders* pMinimap = new UIMiniMapShaders();
	pMinimap->SetNumObject(m_nRoom);
	pMinimap->BuildObjects(pDevice, pCommandList, 1, m_Room);
	pMinimap->SetNowRoom(&m_nowRoom);
	m_ppUIShaders[1] = pMinimap;
}

void TestScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_Camera->UpdateShaderVariables(pCommandList);
	m_pLights->UpdateShaderVariables();
	UpdateShaderVarialbes();

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pLights->LightUploadBuffer()->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(2, d3dcbMaterialsGpuVirtualAddress); //Materials

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbFogGpuVirtualAddress = m_pd3dcbFog->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(5, d3dcbFogGpuVirtualAddress); //Materials

	//플레이어 랜더링
	m_playerShader->Render(pCommandList, m_Camera.get());
	//문 랜더링
	m_gateShader->Render(pCommandList, m_Camera.get());
	//맵, 적, 발사체 랜더링 (방)
	m_Room[m_nowRoom]->Render(pCommandList, m_Camera.get());
	//이펙트 파티클 랜더링
	m_EffectShaders->Render(pCommandList, m_Camera.get());
	m_hitEffectShaders->Render(pCommandList, m_Camera.get());
	//페이트 INOUT 랜더링
	m_pFadeEffectShader->Render(pCommandList, m_Camera.get());

	m_pLights->Render(pCommandList, m_Camera.get());
}

void TestScene::RenderShadow(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList){ }

void TestScene::RenderUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	for (UINT i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Render(pCommandList);
}

void TestScene::CreateShadowMap(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList, int index)
{
	VS_CB_CAMERA_INFO* info = m_pLights->LightMatrix(index);
	LIGHT light = m_pLights->Light(index);
	if (!info)
		return;

	pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	m_Camera->UpdateShaderVariables(pCommandList, *info);
	m_Camera->SetShadowProjection(info->m_xmf4x4ShadowProjection[0], index);
	m_playerShader->RenderToDepthBuffer(pCommandList, m_Camera.get(), light.m_xmf3Position, light.m_fRange);
	m_Room[m_nowRoom]->RenderToDepthBuffer(pCommandList, m_Camera.get(), light.m_xmf3Position, light.m_fRange);
}

void TestScene::CalculateLightMatrix(VS_CB_CAMERA_INFO & cameraInfo, int index, float offset)
{
	
}

bool TestScene::OnKeyboardInput(const Timer& gt, HWND& hWin)
{
	if (m_changeFade > FADE_OFF) {
		m_testPlayer->Movement(NULL);
		return false;
	}
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

	if (GetAsyncKeyState('P') & 0x8000) {
		m_testPlayer->GetStatus()->m_health = 0.0f;
	}
	if (GetAsyncKeyState('T') & 0x8000) {
		m_Room[m_nowRoom]->SetClear(true);
	}
	if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) { //한번만 체크
		if (!m_bMouseCapture) {
			::GetCursorPos(&m_ptOldCursorPos);
			m_bMouseCapture = true;
		}
		else {
			::ReleaseCapture();
			m_bMouseCapture = false;
		}
	}

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		input |= ANI_SKILL;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		input |= ANI_ATTACK;

	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
		cout << m_testPlayer->GetPosition();

	if (m_bMouseCapture)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;

		::SetCapture(hWin);
		::GetCursorPos(&ptCursorPos);
		
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / m_fMouseSensitive;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / m_fMouseSensitive;
		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);

		if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta) {
				OnMouseMove(cxDelta, cyDelta);
			}
		}
	}

	if(!m_testPlayer->Movement(input))
		m_testPlayer->Move(moveInpout, gt.DeltaTime());

	return true;
}

bool TestScene::OnMouseDown(HWND& hWin, WPARAM btnState, UINT nMessageID, int x, int y)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		/*::SetCapture(hWin);
		::GetCursorPos(&m_ptOldCursorPos);
		m_bMouseCapture = true;*/
		if (m_bMouseCapture) {
			DWORD input = 0;
			input |= ANI_ATTACK;
			m_testPlayer->Movement(input);
		}
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
	return true;
}

bool TestScene::OnMouseUp(HWND& hWin, WPARAM btnState, UINT nMessageID, int x, int y)
{
	switch (nMessageID)
	{
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		/*::ReleaseCapture();
		m_bMouseCapture = false;*/
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
	return true;
}

bool TestScene::OnMouseMove(float x, float y)
{
	if (m_changeFade > FADE_OFF)
		return false;
	m_testPlayer->Rotate(y, x, 0.0f);
	return true;
}

void TestScene::RoomChange()
{
	//현재 방번호가 변화시키는 방번호와 같으면 바로 리턴
	if (m_nowRoom == m_isRoomChange.m_roomNum)
		return;

	//방이 변화한다는 플래그가 false 이면 바로 리턴 (오류 처리를 위해 별도 선언)
	if (!m_isRoomChange.m_isChange)
		return;

	//맨처음 시작인 경우는 물리가 적용된 방이 없으므로 방에 물리해제를 할 필요가 없다
	if (m_nowRoom != START_ROOM) {
		m_Room[m_nowRoom]->RegistShader(m_physics, false, START_NON);
	}

	//플레이어 위치 변화 (이동하는 방 문앞으로 이동)
	XMFLOAT3* playerPos;
	playerPos = m_Room[m_isRoomChange.m_roomNum]->RegistShader(m_physics, true, m_isRoomChange.m_dir);
	m_testPlayer->SetPosition(playerPos->x, playerPos->y, playerPos->z);

	//이동한 방에 적이 있을 경우 적의 포인터를 씬으로 가져옴, 클리어 된 방인 경우 무시
	if (m_Room[m_isRoomChange.m_roomNum]->IsEnemy() && !m_Room[m_isRoomChange.m_roomNum]->IsClear()) {
		GlobalVal::getInstance()->setEnemy(m_Room[m_isRoomChange.m_roomNum]->GetEnemyShader()->getObjects(
			*GlobalVal::getInstance()->getNumEnemy()
		));

		*GlobalVal::getInstance()->getRemainEnemy()
			= m_Room[m_isRoomChange.m_roomNum]->GetEnemyShader()->getRemainObjects();
	}
	else
		GlobalVal::getInstance()->setEnemy(nullptr);

	m_Room[m_isRoomChange.m_roomNum]->ResetFire();

	
	//이동한 방에 적이 발사체를 생성할 경우 발사체의 포인터를 씬으로 가져옴, 클리어 된 방인 경우 무시
	if (m_Room[m_isRoomChange.m_roomNum]->IsProjectile() && !m_Room[m_isRoomChange.m_roomNum]->IsClear()) {
		m_Projectile = m_Room[m_isRoomChange.m_roomNum]->GetProjectileShader();
		GlobalVal::getInstance()->setPorjectile(m_Projectile);
	}

	else {
		m_Projectile = nullptr;
		GlobalVal::getInstance()->setPorjectile(nullptr);
	}

	//실제 방이동
	m_nowRoom = m_isRoomChange.m_roomNum;
	//방이동이 완료하였으므로 change플레그를 false로 바꿔줌
	m_isRoomChange.m_isChange = false;
	std::cout << m_nowRoom << endl;
	m_gateShader->SetPositions(m_Room[m_nowRoom]->GetGatePos());
	//페이드 인아웃 설정
	m_changeFade = FADE_IN;
}

void TestScene::RoomSetting()
{
	srand((UINT)time(0));
	UINT sizeX = 7;
	UINT sizeY = 3;
	STAGE::MapGenerator tMap(rand(), 10);
	tMap.SetMap(sizeX, sizeY);

	int** flag = tMap.getCurrentMap().getMapFlags();

	int count = 0;
	m_virtualMap = new int*[sizeY];
	for (int i = 0; i < sizeY; ++i) {
		m_virtualMap[i] = new int[sizeX];
		for (int j = 0; j < sizeX; ++j) {
			m_virtualMap[i][j] = BLANK_ROOM;
			if (flag[i][j] != 0)
				count += 1;
		}
	}
	m_nRoom = count;
	m_Room = new Room*[m_nRoom];

	count = 0;

	for (int i = 0; i < sizeY; ++i) {
		for (int j = 0; j < sizeX; ++j) {
			if (flag[i][j] != 0) {
				m_Room[count] = new Room(flag[i][j] - 1);
				m_Room[count]->m_mapPosX = j;
				m_Room[count]->m_mapPosY = i;
				m_virtualMap[i][j] = count;
				cout << count << "\t" << m_Room[count]->getType() << endl;
				count += 1;
			}
		}
	}
	cout << endl;
	for (int i = 0; i < sizeY; ++i) {
		for (int j = 0; j < sizeX; ++j) {
			cout << m_virtualMap[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;
	UINT nextRoom[4] = { BLANK_ROOM,BLANK_ROOM,BLANK_ROOM,BLANK_ROOM };
	for (int i = 0; i < m_nRoom; ++i) {
		int nowX = m_Room[i]->m_mapPosX;
		int nowY = m_Room[i]->m_mapPosY;
		for (int j = 0; j < m_nRoom; ++j) {
			if (i == j)
				continue;
			if (nowX == m_Room[j]->m_mapPosX) {
				if (m_Room[j]->m_mapPosY == nowY - 1)
					nextRoom[0] = j;
				if (m_Room[j]->m_mapPosY == nowY + 1)
					nextRoom[1] = j;
			}
			if (nowY == m_Room[j]->m_mapPosY) {
				if (m_Room[j]->m_mapPosX == nowX + 1)
					nextRoom[2] = j;
				if (m_Room[j]->m_mapPosX == nowX - 1)
					nextRoom[3] = j;
			}
		}
		m_Room[i]->SetNextRoom(nextRoom);
		for (int k = 0; k < 4; ++k) {
			nextRoom[k] = BLANK_ROOM;
		}
	}
}

void TestScene::RoomFade()
{
	if (m_changeFade > FADE_OFF) {
		if (!m_pFadeEffectShader->IsUsdedFadeEffect()) {
			if (m_changeFade == FADE_IN) {
				m_pFadeEffectShader->SetFadeIn(true, 0.1f, false);
				m_changeFade = FADE_OUT;
			}
			else if (m_changeFade == FADE_OUT) {
				m_pFadeEffectShader->SetFadeIn(false, 1.0f, false);
				m_changeFade = FADE_END;
			}
			else
				m_changeFade = FADE_OFF;
		}
	}
}

void TestScene::BuildLightsAndMaterials()
{

	m_pMaterials = new MATERIALS();
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}

void TestScene::BuildFog()
{
	m_pFog = new CB_FOG_INFO();
	::ZeroMemory(m_pFog, sizeof(CB_FOG_INFO));

	m_pFog->m_xmf4FogColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

	float fFogMode = 1.0f;
	float fStart = 50.0f;
	float fEnd = 500.0f;
	float fDensity = 1.0f; 
	m_pFog->m_xmf4Foginfo = XMFLOAT4(fFogMode, fStart, fEnd, fDensity);	
}

/*
XMFLOAT4				m_xmf4Ambient;	// 앰비언트 반사 색상
XMFLOAT4				m_xmf4Diffuse;
XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
XMFLOAT4				m_xmf4Emissive;
*/
