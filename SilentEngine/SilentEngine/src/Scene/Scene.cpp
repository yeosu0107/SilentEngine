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

void Scene::RoomFade()
{
	if (m_changeFade > FADE_OFF) {
		if (!m_pFadeEffectShader->IsUsdedFadeEffect()) {
			if (m_changeFade == FADE_IN) {
				m_pFadeEffectShader->SetFadeIn(true, m_fFadeInTime, false);
				m_changeFade = FADE_OUT;
			}
			else if (m_changeFade == FADE_OUT) {
				m_pFadeEffectShader->SetFadeIn(false, m_fFadeOutTime, false);
				m_changeFade = FADE_END;
			}
			else
				m_changeFade = FADE_OFF;
		}
	}
}


TestScene::TestScene()
{
	m_physics = new BasePhysX(60.0f);
	m_testPlayer = nullptr;
	m_nowRoom = START_ROOM;
	/*m_isRoomChange = Door(0, START_SOUTH, true);

	RoomSetting();*/
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
	pd3dRootParameters[0].Descriptor.ShaderRegister = CBVCameraInfo; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Game Objects
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = CBVMaterial; //Materials
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = CBVLights; //Lights
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
{ }

void TestScene::UpdateShaderVarialbes() {
	LIGHT_MANAGER->UpdateShaderVariables();
	MATERIAL_MANAGER->UpdateShaderVariables();
}

bool TestScene::Update(const Timer & gt)
{
	m_pFadeEffectShader->Animate(gt.DeltaTime());

	if (m_isGameEnd && m_changeFade == FADE_END) {
		m_isGameEnd = false;
		m_bMouseCapture = true;
		return true;
	}

	for (int i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Animate(gt.DeltaTime());

	RoomChange();	
	RoomFade();		

	if (m_Room[m_nRoom - 1]->IsClear())
		cout << "Clear" << endl;

	if (!m_bMouseCapture) {
		m_pButtons->CollisionButton();
		return false;
	}

	//물리 시물레이트
	m_physics->stepPhysics(false);
	//플레이어 애니메이트
	m_playerShader->Animate(gt.DeltaTime());
	if (m_attackEvent) {
		DWORD input = 0;
		input |= ANI_ATTACK;
		m_testPlayer->Movement(input);
	}
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

	
	return false;
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	//RoomSetting();

	::GetCursorPos(&m_ptOldCursorPos);
	
	globalEffects = GlobalVal::getInstance()->getEffectLoader();
	globalMaps = GlobalVal::getInstance()->getMapLoader();
	loader = GlobalVal::getInstance()->getFirePos();
	

	BuildRootSignature(pDevice, pCommandList);
	CreateShaderVariables(pDevice, pCommandList);

	m_Camera = make_unique<CThirdPersonCamera>();
	m_Camera->InitCamera(pDevice, pCommandList);
	m_Camera->SetOffset(XMFLOAT3(0.0f, 100.0f, -60.0f));
	m_Camera->SetTimeLag(0.30f);
	GlobalVal::getInstance()->setCamera(m_Camera.get());

	PlayerShader* player = new PlayerShader(1, m_Camera.get());
	player->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, m_physics);
	m_playerShader = player;
	
	PaticleShader<PaticleObject>* Explosion = new PaticleShader<PaticleObject>();
	Explosion->SetCamera(m_Camera.get());
	Explosion->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(1));
	m_EffectShaders = Explosion;

	PaticleShader<HitPaticle>* hitShader = new PaticleShader<HitPaticle>();
	hitShader->SetCamera(m_Camera.get());
	hitShader->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(3));
	hitShader->setAnimSpeed(50.0f);
	m_hitEffectShaders = hitShader;

	fireObjectShaders = new PaticleShader<PaticleObject>();
	fireObjectShaders->SetCamera(m_Camera.get());
	fireObjectShaders->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(2));
	fireObjectShaders->setLoop(true);
	fireObjectShaders->setAnimSpeed(50.0f);
	fireObjectShaders->SetRotateLockXZ(true);
	
	map = new InstanceModelShader*[MAX_MAP];
	for (UINT i = 0; i < MAX_MAP; ++i) {
		map[i] = new MapShader(i);
		map[i]->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET);
	}
	/*for (UINT i = 0; i < m_nRoom; ++i) {
		m_Room[i]->SetMapShader(map[m_Room[i]->getType()]);
		m_Room[i]->SetFireShader(fireObjectShaders);
		m_Room[i]->SetFirePosition(loader->getPosition(m_Room[i]->getType()));
		m_Room[i]->SetStartPoint(globalMaps->getStartpoint(m_Room[i]->getType()).returnPoint());
	}*/


	InstanceModelShader* gateShader = new InstanceModelShader(10);
	gateShader->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET);
	gateShader->setPhys(m_physics);
	m_gateShader = gateShader;

	
	enemyShader = new ModelShader*[KindOfEnemy];
	int enemyNum[5] = { 6,3,4,2,1 };

	enemyShader[0] = new EnemyShader<CreepArm>(4);
	enemyShader[1] = new EnemyShader<Enemy>(3);
	enemyShader[2] = new EnemyShader<Ghost>(2);
	enemyShader[3] = new EnemyShader<Skull>(5);
	enemyShader[4] = new EnemyShader<Rich>(6);

	for (int i = 0; i < 5; ++i) {
		enemyShader[i]->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, (int*)enemyNum[i]);
	}

	bullet = new ProjectileShader();
	bullet->SetCamera(m_Camera.get());
	bullet->BuildObjects(pDevice, pCommandList , NUM_RENDERTARGET, globalEffects->getTextureFile(0));
	bullet->setPhys(m_physics);

	m_pFadeEffectShader = new FadeEffectShader();
	m_pFadeEffectShader->BuildObjects(pDevice, pCommandList, 1);

	m_testPlayer = player->getPlayer(0);
	m_testPlayer->GetPosition();
	GlobalVal::getInstance()->setPlayer(m_testPlayer);
	
	GlobalVal::getInstance()->setHitPaticle(m_hitEffectShaders);

	BuildUI(pDevice, pCommandList);
}

void TestScene::BuildUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_ppUIShaders.clear();
	m_nUIShaders = 2;
	m_ppUIShaders.resize(m_nUIShaders);

	UIHPBarShaders* pHPBar = new UIHPBarShaders();
	pHPBar->BuildObjects(pDevice, pCommandList, 1, m_testPlayer);
	m_ppUIShaders[0] = pHPBar;
	UIMiniMapShaders* pMinimap = new UIMiniMapShaders();
	pMinimap->SetNumObject(21);
	pMinimap->BuildObjects(pDevice, pCommandList, 1);
	m_ppUIShaders[1] = pMinimap;

	vector<TextureDataForm> texutredata(2);
	texutredata[0].m_texture = L"res\\Texture\\PauseGame.DDS";
	m_pPauseScreen = new TextureToFullScreen();
	m_pPauseScreen->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);

	texutredata[1].m_texture = L"res\\Texture\\Back.DDS";
	texutredata[0].m_texture = L"res\\Texture\\BackToMainMenu.DDS";
	
	m_pButtons = new UIButtonShaders();
	m_pButtons->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, &texutredata);
	m_pButtons->SetPoint(m_pCursorPos);
	m_pButtons->SetPos(new XMFLOAT2(640.0f, 720.0f - 407.3f), 0);
	m_pButtons->SetPos(new XMFLOAT2(870.0f, 720.0f - 455.0f), 1);
	m_pButtons->SetScale(new XMFLOAT2(1.0f, 1.0f), 0);
	m_pButtons->SetScale(new XMFLOAT2(1.0f, 1.0f), 1);
	m_pButtons->CreateCollisionBox();
}

void TestScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_Camera->UpdateShaderVariables(pCommandList);
	LIGHT_MANAGER->UpdateShaderVariables();
	UpdateShaderVarialbes();

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = LIGHT_MANAGER->LightUploadBuffer()->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = MATERIAL_MANAGER->MaterialUploadBuffer()->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(2, d3dcbMaterialsGpuVirtualAddress); //Materials

	//플레이어 랜더링
	m_playerShader->Render(pCommandList, m_Camera.get());
	//문 랜더링
	m_gateShader->Render(pCommandList, m_Camera.get());
	//맵, 적, 발사체 랜더링 (방)
	m_Room[m_nowRoom]->Render(pCommandList, m_Camera.get());
	//이펙트 파티클 랜더링
	m_EffectShaders->Render(pCommandList, m_Camera.get());
	m_hitEffectShaders->Render(pCommandList, m_Camera.get());
	
	LIGHT_MANAGER->Render(pCommandList, m_Camera.get());
}

void TestScene::RenderShadow(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList){ }

void TestScene::RenderUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	for (UINT i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Render(pCommandList);

	if (!m_bMouseCapture) {
		m_pPauseScreen->Render(pCommandList, m_Camera.get());
		m_pButtons->Render(pCommandList);
	}
	//페이트 INOUT 랜더링
	m_pFadeEffectShader->Render(pCommandList, m_Camera.get());
}

void TestScene::CreateShadowMap(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList, int index)
{
	VS_CB_CAMERA_INFO* info = LIGHT_MANAGER->LightMatrix(index);
	LIGHT light = LIGHT_MANAGER->Light(index);
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
	if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
		if (!m_bMouseCapture) {
			::GetCursorPos(&m_ptOldCursorPos);
			m_bMouseCapture = true;
		}
		else {
			::ReleaseCapture();
			m_bMouseCapture = false;
		}
	}

	if (!m_bMouseCapture)
		return false;

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

	if (GetAsyncKeyState('P') & 0x0001) {
		m_testPlayer->GetStatus()->m_health = 0.0f;
	}
	if (GetAsyncKeyState('T') & 0x0001) {
		m_Room[m_nowRoom]->SetClear(true);
	}

	if (GetAsyncKeyState('Q') & 0x0001) {
		return true;
	}

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		input |= ANI_SKILL;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x0001)
		input |= SUPER_SPEED;


	if (GetAsyncKeyState(VK_RETURN) & 0x0001)
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

	return false;
}

bool TestScene::OnMouseDown(HWND& hWin, WPARAM btnState, UINT nMessageID, int x, int y)
{
	UINT collButon = 0;
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		/*::SetCapture(hWin);
		::GetCursorPos(&m_ptOldCursorPos);
		m_bMouseCapture = true;*/
		if (m_bMouseCapture) {
			/*DWORD input = 0;
			input |= ANI_ATTACK;
			m_testPlayer->Movement(input);*/
			m_attackEvent = true;
		}

		else {
			m_attackEvent = false;
			collButon = m_pButtons->CollisionButton();

			if (collButon == 1) {
				m_changeFade = FADE_IN;
				m_isGameEnd = true;
				m_Room[m_nowRoom]->RegistShader(m_physics, false, START_NON);
			}
			else if (collButon == 2) {
				::GetCursorPos(&m_ptOldCursorPos);
				m_bMouseCapture = true;
			}
			return false;
		}
		break;
	case WM_RBUTTONDOWN:
	{
		DWORD input = 0;
		input |= ANI_KICK;
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
		m_attackEvent = false;
		/*::ReleaseCapture();
		m_bMouseCapture = false;*/
	case WM_RBUTTONUP:
		
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

	// 프레임워크에서 넘어온 값을 제대로 처리하기 위해 한번 더 판별
	if (!m_bMouseCapture) {
		m_pCursorPos->x = x;
		m_pCursorPos->y = y;
	}
	else
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

	bool fogEnabled = GlobalVal::getInstance()->getMapLoader()->getFogEnabled(m_Room[m_isRoomChange.m_roomNum]->getType());
	GlobalVal::getInstance()->setFogEnable(fogEnabled);

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
	if (m_Room) {
		for (int i = 0; i < m_nRoom; ++i)
			delete m_Room[i];

		delete[] m_Room;
	}

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

	for (UINT i = 0; i < m_nRoom; ++i) {
		m_Room[i]->SetMapShader(map[m_Room[i]->getType()]);
		m_Room[i]->SetFireShader(fireObjectShaders);
		m_Room[i]->SetFirePosition(loader->getPosition(m_Room[i]->getType()));
		m_Room[i]->SetStartPoint(globalMaps->getStartpoint(m_Room[i]->getType()).returnPoint());
		UINT index = i % (KindOfEnemy - 1);
		m_Room[i]->SetEnemyShader(enemyShader[index]);
		if (index >= 2)
			m_Room[i]->SetProjectileShader(bullet);
	}
	m_Room[m_nRoom - 1]->SetEnemyShader(enemyShader[4]);
	m_Room[m_nRoom - 1]->SetProjectileShader(bullet);
}

void TestScene::CaptureCursor()
{
	::GetCursorPos(&m_ptOldCursorPos);
}

void TestScene::ResetScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_isRoomChange = Door(0, START_SOUTH, true);
	m_nowRoom = START_ROOM;
	RoomSetting();
	RoomChange();
	//BuildUI(pDevice, pCommandList);
	auto minimap = reinterpret_cast<UIMiniMapShaders*>(m_ppUIShaders[1]);
	minimap->SetNumObject(m_nRoom);
	minimap->setRoomPos(m_Room);
	minimap->SetNowRoom(&m_nowRoom);
	m_changeFade = FADE_OUT;
	reinterpret_cast<Player*>(m_testPlayer)->reset();
}



bool MainScene::Update(const Timer & gt)
{
	m_pButtons->CollisionButton();
	m_pFadeEffectShader->Animate(gt.DeltaTime());
	RoomFade();

	if (m_changeFade == FADE_END) {
		if(m_isGameEnd)  
			PostQuitMessage(0);
		
		m_changeFade = FADE_OFF;
		return true;
	}
	return false;
}

void MainScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Camera = make_unique<CThirdPersonCamera>();
	m_Camera->InitCamera(pDevice, pCommandList);
	m_Camera->SetOffset(XMFLOAT3(0.0f, 100.0f, -60.0f));
	m_Camera->SetTimeLag(0.30f);

	vector<TextureDataForm> texutredata(2);
	texutredata[1].m_texture = L"res\\MainSceneTexture\\GameExit.dds";
	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameStart.dds";
	m_pButtons = new UIButtonShaders();
	m_pButtons->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, &texutredata);
	m_pButtons->SetPoint(m_pCursorPos);

	texutredata[0].m_texture = L"res\\MainSceneTexture\\MainBackgound_COLOR.dds";
	m_pBackground = new TextureToFullScreen();
	m_pBackground->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, &texutredata[0]);

	m_pFadeEffectShader = new FadeEffectShader();
	m_pFadeEffectShader->BuildObjects(pDevice, pCommandList, 1);

	m_fFadeInTime = 1.0f;
	m_fFadeOutTime = 1.0f;
	m_bMouseCapture = false;
}

void MainScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	
}

void MainScene::RenderUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pBackground->Render(pCommandList, m_Camera.get());
	m_pButtons->Render(pCommandList);
	m_pFadeEffectShader->Render(pCommandList, m_Camera.get());
}

bool MainScene::OnMouseDown(HWND & hWin, WPARAM btnState, UINT nMessageID, int x, int y)
{
	UINT collButon = m_pButtons->CollisionButton();
	if (collButon == GAME_START) {
		m_changeFade = FADE_IN;
		return true;
	}
	else if (collButon == GAME_END) {
		m_changeFade = FADE_IN;
		m_isGameEnd = true;
	}
	return false;
}

bool MainScene::OnKeyboardInput(const Timer & gt, HWND & hWin)
{
	return false;
}

bool MainScene::OnMouseMove(float x, float y)
{
	m_pCursorPos->x = x;
	m_pCursorPos->y = y;
	return false;
}

