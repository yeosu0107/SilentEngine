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
	m_BonusShader->RefreshTimer(gt.DeltaTime());

	m_SkilCooldown->Animate(gt.DeltaTime());
	for (int i = NUM_KICK; i <= NUM_AVOID; ++i) {
		m_NumberShader[i]->SetData(m_SkilCooldown->RemainingCooldown(i - 2), 0.0f);
	}
	m_pFadeEffectShader->Animate(gt.DeltaTime());
	m_pTakeDamageScreen->Animate(gt.DeltaTime());

	if (m_isGameEnd && m_changeFade == FADE_END) {
		m_isGameEnd = false;
		m_bMouseCapture = true;
		return true;
	}

	for (int i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Animate(gt.DeltaTime());


	if(m_Room[m_nowRoom] != nullptr)
		m_MonsterHP->LinkedMonster(m_Room[m_nowRoom]->GetEnemyShader()->getObjects(OPTSETALL), m_Room[m_nowRoom]->getNumEnemy());
	m_MonsterHP->Animate(gt.DeltaTime());

	RoomChange();	
	RoomFade();		

	//물리 시물레이트
	m_physics->stepPhysics(false);
	//플레이어 애니메이트
	m_playerShader->Animate(gt.DeltaTime());

	m_nSceneState = static_cast<SceneType>(CalNowScene());
	if (m_nSceneState != NORMALLY) {
		m_pUIScenes[m_nSceneState]->ColiisionToMouseMove(m_pCursorPos);
		return m_pUIScenes[m_nSceneState]->Update(gt);
	}

	if (m_attackEvent) {
		DWORD input = 0;
		input |= ANI_ATTACK;
		m_testPlayer->Movement(input);
	}
	//게이트 애니메이트(문 열리는 애니메이션, 방 클리어 시만 수행)
	if (m_Room[m_nowRoom]->IsClear()) {
		if(m_Room[m_nowRoom]->getType() != BOSS_ROOM)
			m_gateShader->Animate(gt.DeltaTime(), m_Room[m_nowRoom]->getNextRoom());

		//클리어 보너스
		if (m_Room[m_nowRoom]->IsStatBouns()) {
			m_Room[m_nowRoom]->SetStatBouns(false);
			//reinterpret_cast<Player*>(m_testPlayer)->roomClearBouns(ClearBouns::plusAtk);
			UINT index = reinterpret_cast<Player*>(m_testPlayer)->roomClearBouns();
			m_BonusShader->SetNowSprite(XMUINT2(0, index));

			SoundMgr::getInstance()->play(SOUND::ROOM_CLEAR, CHANNEL::FX);
		}
	}

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
	int type;
	XMFLOAT3 pos[2];
	if (GlobalVal::getInstance()->isDrawPaticle(type, pos)) {
		m_hitEffectShaders[type]->SetPos(pos, 2);
	}
	for (auto&p : m_hitEffectShaders) {
		p->Animate(gt.DeltaTime());
	}
	//m_hitEffectShaders->Animate(gt.DeltaTime());

	
	return false;
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	//RoomSetting();
	//마우스 초기화 관련
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
	hitShader->setPaticleSize(100.0f);
	hitShader->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(3));
	hitShader->setAnimSpeed(50.0f);
	m_hitEffectShaders.emplace_back(hitShader);
	//m_hitEffectShaders = hitShader;

	PaticleShader<HitPaticle>* hitShader2 = new PaticleShader<HitPaticle>();
	hitShader2->SetCamera(m_Camera.get());
	hitShader2->setPaticleSize(150.0f);
	hitShader2->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(4));
	hitShader2->setAnimSpeed(50.0f);
	m_hitEffectShaders.emplace_back(hitShader2);

	PaticleShader<HitPaticle>* hitShader3 = new PaticleShader<HitPaticle>();
	hitShader3->SetCamera(m_Camera.get());
	hitShader3->setPaticleSize(150.0f);
	hitShader3->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(5));
	hitShader3->setAnimSpeed(50.0f);
	m_hitEffectShaders.emplace_back(hitShader3);

	PaticleShader<HitPaticle>* hitShader4 = new PaticleShader<HitPaticle>();
	hitShader4->SetCamera(m_Camera.get());
	hitShader4->setPaticleSize(300.0f);
	hitShader4->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, globalEffects->getTextureFile(6));
	hitShader4->setAnimSpeed(25.0f);
	m_hitEffectShaders.emplace_back(hitShader4);

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

	InstanceModelShader* gateShader = new InstanceModelShader(11);
	gateShader->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET);
	gateShader->setPhys(m_physics);
	m_gateShader = gateShader;

	
	enemyShader = new ModelShader*[KindOfEnemy];
	int enemyNum[7] = { 10,10,10,10,10,10,1 };

	enemyShader[0] = new EnemyShader<CreepArm>(4);
	enemyShader[1] = new EnemyShader<Enemy>(3);
	enemyShader[2] = new EnemyShader<Enemy>(0);
	enemyShader[3] = new EnemyShader<Enemy>(7);
	enemyShader[4] = new EnemyShader<Ghost>(2);
	enemyShader[5] = new EnemyShader<Skull>(5);
	enemyShader[6] = new EnemyShader<Rich>(6);

	for (int i = 0; i < KindOfEnemy; ++i) {
		enemyShader[i]->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, (int*)enemyNum[i]);
	}
	enemyShader[2]->setScale(1.5f);
	enemyShader[3]->setScale(2.0f);

	bullet = new ProjectileShader();
	bullet->SetCamera(m_Camera.get());
	bullet->BuildObjects(pDevice, pCommandList , NUM_RENDERTARGET, globalEffects->getTextureFile(0));
	bullet->setPhys(m_physics);

	m_pFadeEffectShader = new FadeEffectShader();
	m_pFadeEffectShader->BuildObjects(pDevice, pCommandList, 1, NULL);

	m_testPlayer = player->getPlayer(0);
	m_testPlayer->GetPosition();
	GlobalVal::getInstance()->setPlayer(m_testPlayer);
	m_physics->registerPlayer(m_testPlayer);

	//GlobalVal::getInstance()->setHitPaticle(m_hitEffectShaders);

	BuildUI(pDevice, pCommandList);
}

void TestScene::BuildUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	Player* player = dynamic_cast<Player*>(m_playerShader->getPlayer(0));
	m_ppUIShaders.clear();
	m_nUIShaders = 3;
	m_ppUIShaders.resize(m_nUIShaders);

	m_MonsterHP = new MonsterHPShaders();
	m_MonsterHP->BuildObjects(pDevice, pCommandList, 2, globalEffects->getTextureFile(0));
	m_MonsterHP->SetCamera(m_Camera.get());

	vector<TextureDataForm> texutredata(2);

	UIHPBarShaders* pHPBar = new UIHPBarShaders();
	pHPBar->BuildObjects(pDevice, pCommandList, 1, m_testPlayer);
	m_ppUIShaders[0] = pHPBar;

	UIMiniMapShaders* pMinimap = new UIMiniMapShaders();
	pMinimap->SetNumObject(21);
	pMinimap->BuildObjects(pDevice, pCommandList, 1);
	m_ppUIShaders[1] = pMinimap;

	texutredata[0].m_texture = L"res\\MainSceneTexture\\Cooldown.DDS";
	texutredata[0].m_MaxX = 4;

	m_SkilCooldown = make_unique<SkillUIShaders>();
	m_SkilCooldown->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	m_SkilCooldown->LinkedSkillTime(player->getKickDelay(),  5000, player->getCoolTimeBonus(), 0);
	m_SkilCooldown->LinkedSkillTime(player->getUpperDelay(), 5000, player->getCoolTimeBonus(), 1);
	m_SkilCooldown->LinkedSkillTime(player->getPunchDelay(), 5000, player->getCoolTimeBonus(), 2);
	m_SkilCooldown->LinkedSkillTime(player->getAvoidDelay(), 5000, player->getCoolTimeBonus(), 3);

	m_SkilCooldown->SetScale(&XMFLOAT2(0.7f, 0.7f), OPTSETALL);
	m_SkilCooldown->SetPosScreenRatio(XMFLOAT2(0.5f, 0.045f), OPTSETALL);
	m_SkilCooldown->MovePos(XMFLOAT2(-73.0f, 0.0f), 0);
	m_SkilCooldown->MovePos(XMFLOAT2(71.0f, 0.0f), 2);
	m_SkilCooldown->MovePos(XMFLOAT2(146.0f, 0.0f), 3);
	
	m_BonusShader = make_unique<UIShaders>();
	texutredata[0] = { L"res\\Texture\\EventMessage.DDS", L"", 1 , 7 };
	m_BonusShader->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	m_BonusShader->SetPosScreenRatio(XMFLOAT2(0.5f, 0.8f));
	m_BonusShader->SetTimer(0.5f, 2.0f, 0.5f);
	m_BonusShader->SetEnable(false);

	UIShaders* pSkill = new UIShaders();	
	texutredata[0] = { L"res\\Texture\\Skill.DDS", L"", 1, 1 };

	pSkill->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	pSkill->SetPosScreenRatio(XMFLOAT2( 0.5f, 0.045f));
	m_ppUIShaders[2] = pSkill;

	texutredata[0].m_texture = L"res\\Texture\\blood4_bullet.DDS";
	m_pTakeDamageScreen = new FadeEffectShader();
	m_pTakeDamageScreen->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);

	/*texutredata[0].m_texture = L"res\\Texture\\dash.DDS";
	m_SuperRunEffect = make_unique<TextureToFullScreen>();
	m_SuperRunEffect->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);*/

	BuildNumberUI(pDevice, pCommandList);

	
	m_pUIScenes = new virtualScene*[m_nUIScenes];

	PauseScene* pauseScene = new PauseScene();
	pauseScene->BuildScene(pDevice, pCommandList);
	pauseScene->SetPoint(m_pCursorPos);
	m_pUIScenes[PAUSE] = pauseScene;

	GameOverScene* gameOverScene = new GameOverScene();
	gameOverScene->BuildScene(pDevice, pCommandList);
	m_pUIScenes[GAMEOVER] = gameOverScene;

	ClearScene* clearScene = new ClearScene();
	clearScene->BuildScene(pDevice, pCommandList);
	m_pUIScenes[CLEAR] =clearScene;
}

void TestScene::BuildNumberUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	Player* player = dynamic_cast<Player*>(m_playerShader->getPlayer(0));
	vector<TextureDataForm> texutredata(2);

	texutredata[0].m_texture = L"res\\Texture\\Number_WT.DDS";
	
	XMFLOAT2 pos = XMFLOAT2(
		static_cast<float>(FRAME_BUFFER_WIDTH) / 6.0 + 100.0f,
		static_cast<float>(FRAME_BUFFER_HEIGHT) * 8.0f / 9.0 + 38.0f
	);

	m_NumberShader = new NumberUIShaders*[m_nNumberShader];
	Status* stat = player->GetStatus();
	NumberUIShaders* HPnumShader = new NumberUIShaders();
	HPnumShader->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	HPnumShader->SetNumberInfo(NUM_TYPE_UINT_DIVISION, 7);
	HPnumShader->LinkData(&stat->m_health, &stat->m_maxhealth);
	HPnumShader->SetScale(&XMFLOAT2(0.56f * 4, 0.36f), OPTSETALL);
	HPnumShader->SetPos(&pos, 0);

	m_NumberShader[NUM_HP] = HPnumShader;

	pos.y -= 22.0f;
	NumberUIShaders* MPnumShader = new NumberUIShaders();
	MPnumShader->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	MPnumShader->SetNumberInfo(NUM_TYPE_UINT_DIVISION, 7);
	MPnumShader->LinkData(&stat->m_mp, &stat->m_maxmp);
	MPnumShader->SetScale(&XMFLOAT2(0.56f * 4, 0.36f), OPTSETALL);
	MPnumShader->SetPos(&pos, 0);

	m_NumberShader[NUM_MP] = MPnumShader;

	pos = XMFLOAT2(
		static_cast<float>(FRAME_BUFFER_WIDTH) / 6.0 + 150.0f,
		static_cast<float>(FRAME_BUFFER_HEIGHT) * 8.0f / 9.0 - 38.0f
	);

	for (int i = NUM_KICK; i <= NUM_AVOID; ++i) {

		NumberUIShaders* SkillnumShader = new NumberUIShaders();
		SkillnumShader->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
		SkillnumShader->SetNumberInfo(NUM_TYPE_FLOAT_NONE, 3);
		SkillnumShader->SetData(m_SkilCooldown->RemainingCooldown(i - 2), 0.0f);
		SkillnumShader->SetScale(&XMFLOAT2(0.7f, 0.7f), OPTSETALL);
		SkillnumShader->SetPosScreenRatio(XMFLOAT2(0.5f, 0.045f), OPTSETALL);
		SkillnumShader->MovePos(XMFLOAT2(- 76.0f + (73.0f * (i - 2)), 0.0f), 0);
		SkillnumShader->SetScale(&XMFLOAT2(0.5f * 2, 0.36f), OPTSETALL);

		m_NumberShader[i] = SkillnumShader;
	}
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
	if(m_Room[m_nowRoom]->getType() != BOSS_ROOM)
		m_gateShader->Render(pCommandList, m_Camera.get());
	//맵, 적, 발사체 랜더링 (방)
	m_Room[m_nowRoom]->Render(pCommandList, m_Camera.get());
	//이펙트 파티클 랜더링
	m_EffectShaders->Render(pCommandList, m_Camera.get());
	for (auto& p : m_hitEffectShaders) {
		p->Render(pCommandList, m_Camera.get());
	}
	//m_hitEffectShaders->Render(pCommandList, m_Camera.get());
	
	LIGHT_MANAGER->Render(pCommandList, m_Camera.get());
	
}

void TestScene::RenderHPBars(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_MonsterHP->Render(pCommandList, m_Camera.get());
}

void TestScene::RenderShadow(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList){ }

void TestScene::RenderUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{

	// 플레이어가 죽었는지 탐지
	if (m_testPlayer->GetStatus()->m_health <= 0.0f && !m_bIsGameOver) {
		m_bIsGameOver = true;
		::ReleaseCapture();
		m_bMouseCapture = false;
	}
	
	if(m_Room[m_nowRoom]->getType() == BOSS_ROOM && m_Room[m_nowRoom]->IsClear()){
		m_bIsClear = true;
		::ReleaseCapture();
		m_bMouseCapture = false;
	}

	if (m_testPlayer->getHitted() == true) {
		if(m_testPlayer->isLive() == true)
			m_pTakeDamageScreen->SetFadeIn(true, 0.2f, true, XMFLOAT3(1.0f, 1.0f, 1.0f));
		m_testPlayer->setHitted(false);
	}

	/*if (m_testPlayer->getDash()) {
		m_SuperRunEffect->Render(pCommandList, m_Camera.get());
	}*/

	m_BonusShader->Render(pCommandList);
	m_pTakeDamageScreen->Render(pCommandList, m_Camera.get());
	
	for (UINT i = 0; i < m_nUIShaders; ++i)
		m_ppUIShaders[i]->Render(pCommandList);
	m_SkilCooldown->Render(pCommandList);
	for(UINT i = 0; i < m_nNumberShader; ++i)
		m_NumberShader[i]->Render(pCommandList);

	m_nSceneState = static_cast<SceneType>(CalNowScene());
	if (m_nSceneState != NORMALLY) {
		m_pUIScenes[m_nSceneState]->Render(pDevice, pCommandList);
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
		if (!m_bMouseCapture && CalNowScene() == NORMALLY) {
			::GetCursorPos(&m_ptOldCursorPos);
			m_bMouseCapture = true;
			m_nSceneState = static_cast<SceneType>(CalNowScene());
		}
		else if (CalNowScene() == PAUSE) {
			m_bMouseCapture = true;
		}
		else {
			::ReleaseCapture();
			m_bMouseCapture = false;
			m_nSceneState = static_cast<SceneType>(CalNowScene());
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
#ifdef _DEBUG
	if (GetAsyncKeyState('T') & 0x0001) {
		m_Room[m_nowRoom]->SetClear(true);
	}
	
	if (GetAsyncKeyState(VK_RETURN) & 0x0001)
		cout << m_testPlayer->GetPosition();
#endif
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		input |= SUPER_SPEED;

	if (GetAsyncKeyState('Q') & 0x0001) {
		input |= ANI_UPPER;
	}

	if (GetAsyncKeyState('E') & 0x0001) {
		input |= ANI_PUNCH;
	}

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		input |= ANI_SKILL;
	}

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

			//collButon = m_bIsGameOver ? m_pGameOverScene->CollisionToMouse(m_pCursorPos) : m_pPauseScene->CollisionToMouse(m_pCursorPos);
			m_nSceneState = static_cast<SceneType>(CalNowScene());
			collButon = TranslateButton((m_nSceneState != NORMALLY) ? m_pUIScenes[m_nSceneState]->CollisionToMouseClick(m_pCursorPos) : 0);
			ActiveButton(collButon);
			
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
		//*GlobalVal::getInstance()->getRemainEnemy() = m_Room[m_isRoomChange.m_roomNum].get
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
	//std::cout << m_nowRoom << endl;
	
	XMFLOAT3* roomPos = m_Room[m_nowRoom]->GetGatePos();

	for (UINT i = 0; i < 4; ++i)
		LIGHT_MANAGER->SetPosition(roomPos[i], LightManagement::SPOTLIGHT_START + i, XMFLOAT3(0.0f, 120.0f, 0.0f));
	m_gateShader->SetPositions(roomPos);
	//페이드 인아웃 설정
	m_changeFade = FADE_IN;
	if (m_Room[m_nowRoom]->getType() != BOSS_ROOM)
		SoundMgr::getInstance()->changeSound(getRandomBGM(), CHANNEL::BGM);
	else
		SoundMgr::getInstance()->changeSound(SOUND::BOSS, CHANNEL::BGM);
}

void TestScene::RoomSetting()
{
	if (m_Room) {
		for (int i = 0; i < m_nRoom; ++i) {
			m_Room[i]->RegistShader(m_physics, false, START_NON);
			delete m_Room[i];
		}
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
#ifdef _DEBUG
				cout << count << "\t" << m_Room[count]->getType() << endl;
#endif
				count += 1;
			}
		}
	}
	
	m_Room[count - 1]->setType(10);
#ifdef _DEBUG
	cout << endl;
	for (int i = 0; i < sizeY; ++i) {
		for (int j = 0; j < sizeX; ++j) {
			cout << m_virtualMap[i][j] << "\t";
		}
		cout << endl;
	}
	cout << endl;
#endif
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
		m_Room[i]->SetSpawnPoint(globalMaps->getNumOfSpawn(m_Room[i]->getType()), globalMaps->getSpawnPoint(m_Room[i]->getType()));
		//UINT index = i % (KindOfEnemy - 1);
		UINT index = rand() % (KindOfEnemy - 1); //방에 적 스폰 랜덤하게 설정
		//방에 출현하는 적 보정 
		//(5번, 9번 방에서만 5번 몬스터가 스폰됨. 이외에 방에서 5번이 나오면 너무 어려움)
		UINT tmpType = m_Room[i]->getType();
		if ( (tmpType != 5 || tmpType !=9)
			&& index == 5) {
			if (i > 5)
				index -= 2;
			else
				index -= 1;
		}
		if (tmpType == 5 || tmpType == 9) index = 5;

		//방에 적 등록
		m_Room[i]->SetEnemyShader(enemyShader[index]);
		if (index >= 4)
			m_Room[i]->SetProjectileShader(bullet);
	}
	//첫번째 방은 무조건 팔이 나옴
	m_Room[0]->SetEnemyShader(enemyShader[0]);
	m_Room[0]->SetProjectileShader(nullptr);

	//보스룸 설정
	m_Room[m_nRoom - 1]->SetEnemyShader(enemyShader[KindOfEnemy-1]); 
	m_Room[m_nRoom - 1]->SetProjectileShader(bullet);
}

UINT TestScene::TranslateButton(UINT nbutton)
{
	switch (m_nSceneState) {
	case PAUSE:
		switch (nbutton) {
		case 0: return NONE;
		case 1: return BLOOMONOFF;
		case 2: return HDRONOFF;
		case 3: return CONTINUE;
		case 4: return EXIT;
		} break;
	
	case GAMEOVER:
	case CLEAR:
		switch (nbutton) {
		case 0: return NONE;
		case 1: return EXIT;
		} break;

	case NORMALLY:
		break;
	}
	//NONE, EXIT, CONTINUE, HDRONOFF, BLOOMONOFF
	return NONE;
}

void TestScene::ActiveButton(UINT nbuttonType)
{
	//NONE, EXIT, CONTINUE, HDRONOFF, BLOOMONOFF
	switch (nbuttonType) {
	case NONE:
		return;

	case EXIT:
		m_changeFade = FADE_IN;
		m_isGameEnd = true;
		m_Room[m_nowRoom]->RegistShader(m_physics, false, START_NON);
		return;

	case CONTINUE: 
		::GetCursorPos(&m_ptOldCursorPos);
		m_bMouseCapture = true;
		m_nSceneState = static_cast<SceneType>(CalNowScene());
		return;

	case HDRONOFF:
		HDR_ON = !HDR_ON;
		return;

	case BLOOMONOFF:
		BLOOM_ON = !BLOOM_ON;
		return;
	}

	return;
}


UINT TestScene::CalNowScene()
{
	if (m_bIsGameOver) return GAMEOVER;
	if (m_bIsClear) return CLEAR;
	if (!m_bMouseCapture) return PAUSE;
	return NORMALLY;
}

void TestScene::CaptureCursor()
{
	::GetCursorPos(&m_ptOldCursorPos);
}

void TestScene::ResetScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_isRoomChange = Door(0, START_SOUTH, true);
	for (int i = 0; i < m_nUIScenes; ++i) {
		m_pUIScenes[i]->Reset();
	}
	m_bIsGameOver = false;
	m_bIsClear = false;

	m_nowRoom = START_ROOM;
	RoomSetting();
	RoomChange();
	//BuildUI(pDevice, pCommandList);

	auto minimap = reinterpret_cast<UIMiniMapShaders*>(m_ppUIShaders[1]);
	minimap->SetNumObject(m_nRoom);
	minimap->setRoomPos(m_Room);
	minimap->SetNowRoom(&m_nowRoom);
	m_changeFade = FADE_OUT;

	if (!m_bMouseCapture) {
		::GetCursorPos(&m_ptOldCursorPos);
		m_bMouseCapture = true;
	}

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
	texutredata[1].m_MaxX = 2.0f;
	texutredata[1].m_MaxY = 1.0f;

	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameStart.dds";
	texutredata[0].m_MaxX = 2.0f;
	texutredata[0].m_MaxY = 1.0f;

	m_pButtons = new UIButtonShaders();
	m_pButtons->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, &texutredata);
	m_pButtons->SetPoint(m_pCursorPos);
	m_pButtons->SetScale(&XMFLOAT2(1.5f, 1.5f), OPTSETALL);

	texutredata[0].m_texture = L"res\\MainSceneTexture\\MainBackgound_COLOR.dds";
	m_pBackground = new TextureToFullScreen();
	m_pBackground->BuildObjects(pDevice, pCommandList, NUM_RENDERTARGET, &texutredata[0]);

	m_pFadeEffectShader = new FadeEffectShader();
	m_pFadeEffectShader->BuildObjects(pDevice, pCommandList, 1);

	m_fFadeInTime = 1.0f;
	m_fFadeOutTime = 1.0f;
	m_bMouseCapture = false;

	SoundMgr::getInstance()->play(SOUND::MAIN, CHANNEL::BGM);
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

