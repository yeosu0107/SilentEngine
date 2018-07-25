#include "stdafx.h"
#include "Player.h"
#include <Windows.h>

const float startSpeed = 80.0f;
const float accelSpeed = 0.5f;
const float maxSpeed = 100.0f;

Player::Player(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
	: ModelObject(model, pd3dDevice, pd3dCommandList)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pCamera = nullptr;
	m_Callback.SetJump(&m_Jump);

	m_moveSpeed = startSpeed;
	m_playerLogic = new PlayerLogic(this);
	m_playerLogic->setFunc();
	m_status = m_playerLogic->getStatus();
}

Player::~Player()
{
	if (m_pCamera)
		delete m_pCamera;

	ModelObject::~ModelObject();
}

void Player::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		/*if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }*/
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
	/*	if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;*/
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		/*if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }*/
	}
	if (m_pCamera) {
		m_pCamera->Rotate(x, y, z);
		//m_pCamera->Rotate(0, y, 0); //y축 회전만 허용
	}
}

void Player::RegenerateMatrix()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._12= m_xmf3Right.y; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._13 = m_xmf3Right.z; m_xmf4x4World._23 = m_xmf3Up.z; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;

	//스케일 0.5배
	//XMStoreFloat4x4(&m_xmf4x4World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMLoadFloat4x4(&m_xmf4x4World));
	GameObject::Rotate(0, 180, 0); //정면 쳐다보기
	
}

void Player::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	//m_ani[PlayerAni::Idle]->SetAnimSpeed(1.0f);
	//m_ani[PlayerAni::Move]->SetAnimSpeed(0.5f);
	m_ani[PlayerAni::Attack]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::Attack2]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::Attack3]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::Skill]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::KickAttack]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::KickAttack2]->SetAnimSpeed(1.5f);
	m_ani[PlayerAni::PowerPunch]->SetAnimSpeed(1.5f);
	m_ani[PlayerAni::Hitted]->SetAnimSpeed(2.0f);
	m_ani[PlayerAni::die]->DisableLoof(PlayerAni::die);
	m_ani[PlayerAni::Idle]->EnableLoof();
}

bool Player::Move(DWORD input, float fTime)
{
	if (m_playerLogic->getState() > STATE::tracking)
		return false;
	if (input){
		float fDist = m_moveSpeed * fTime;
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		XMFLOAT3 tmpLook=XMFLOAT3(0,0,0);

		//회전 계산
		if (input & DIR_FORWARD) {
			tmpLook = m_pCamera->GetLookVector();
		}
		if (input & DIR_BACKWARD) {
			if (Vector3::IsZero(tmpLook))
				tmpLook = Vector3::ScalarProduct(m_pCamera->GetLookVector(), -1);
			else
				tmpLook = Vector3::Add(tmpLook, Vector3::ScalarProduct(m_pCamera->GetLookVector(), -1), 1);
		}
		if (input & DIR_RIGHT) {
			if (Vector3::IsZero(tmpLook))
				tmpLook = m_pCamera->GetRightVector();
			else
				tmpLook = Vector3::Add(tmpLook, m_pCamera->GetRightVector(), 1);
		}
		if (input & DIR_LEFT) {
			if (Vector3::IsZero(tmpLook))
				tmpLook = Vector3::ScalarProduct(m_pCamera->GetRightVector(), -1);
			else
				tmpLook = Vector3::Add(tmpLook, Vector3::ScalarProduct(m_pCamera->GetRightVector(), -1), 1);
		}

		//플레이어 룩벡터 적용
		if (!Vector3::IsZero(tmpLook)) {
			tmpLook = Vector3::Normalize(tmpLook);
			CalibrateLook(tmpLook);
		}
		else
			return false; //룩벡터가 0인경우는 이동X, false 리턴

		//이동 거리 계산
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDist);

		//가속 처리
		//m_moveSpeed = 80.0f;
		//if (m_moveSpeed < maxSpeed)
		//	m_moveSpeed += accelSpeed;
		
		if (m_Controller) {
			m_Controller->move(XMtoPX(xmf3Shift), 0.001f, 1, m_ControllerFilter);
			//실제 게임 오브젝트의 이동은 애니메에트에서 처리 (현재는 물리적 이동만 처리)
		}
		//m_AnimIndex = PlayerAni::Move;
		//ChangeAnimation(PlayerAni::Move);
		m_playerLogic->changeState(STATE::tracking);
		return true;
	}
	//m_moveSpeed = startSpeed;
	//ChangeAnimation(PlayerAni::Idle);
	m_playerLogic->changeState(STATE::idle);
	return false;
}

bool Player::Movement(DWORD input)
{
	if (input & ANI_ATTACK) {
		if (m_AnimIndex == PlayerAni::Attack)
			m_playerLogic->changeState(STATE::attack2);
		else if (m_AnimIndex == PlayerAni::Attack2)
			m_playerLogic->changeState(STATE::attack3);
		else
			m_playerLogic->changeState(STATE::attack);
	}
	if (input & ANI_SKILL) {
		if (GetTickCount() - m_avoidDelay > (MAX_SKILL_DELAY - m_skillCostBouns)) {
			m_playerLogic->changeState(STATE::skill);
			m_avoidDelay = GetTickCount();
		}
		
	}
	if (input & ANI_KICK) {
		if (GetTickCount() - m_kickDelay > (MAX_SKILL_DELAY - m_skillCostBouns)) {
			if (m_status->m_mp >= (1 - m_mpCostBouns)) {
				m_status->m_mp -= (1 - m_mpCostBouns);
				m_playerLogic->changeState(STATE::kick);
				m_kickDelay = GetTickCount();
			}
#ifdef _DEBUG
			else
				cout << "mp lack : " << m_status->m_mp << endl;
#endif
		}
	}
	if (input & ANI_UPPER) {
		if (GetTickCount() - m_kick2Delay > (MAX_SKILL_DELAY - m_skillCostBouns)) {
			if (m_status->m_mp >= (2 - m_mpCostBouns)) {
				m_status->m_mp -= (2 - m_mpCostBouns);
				m_playerLogic->changeState(STATE::upper);
				m_kick2Delay = GetTickCount();
			}
#ifdef _DEBUG
			else
				cout << "mp lack : " << m_status->m_mp << endl;
#endif
		}
	}

	if (input & ANI_PUNCH) {
		if (GetTickCount() - m_punchDelay > (MAX_SKILL_DELAY - m_skillCostBouns)) {
			if (m_status->m_mp >= (3 - m_mpCostBouns)) {
				m_status->m_mp -= (3 - m_mpCostBouns);
				m_playerLogic->changeState(STATE::punch);
				m_punchDelay = GetTickCount();
			}
#ifdef _DEBUG
			else
				cout << "mp lack : " << m_status->m_mp << endl;
#endif
		}
	}

	if (input & SUPER_SPEED) {
		if(m_moveSpeed<100.0f)
			m_moveSpeed = 320.0f;
		else
			m_moveSpeed = 80.0f;
	}
	if (input != 0)
		return true;

	return false;
}

void Player::Idle()
{
	hitBackstep = 0.0f;
}

void Player::Attack()
{
	//if (m_AnimIndex != PlayerAni::Attack || m_AnimIndex != PlayerAni::Attack2)
	//	return;
	if (m_loopCheck == LOOP_TRIGGER) {
		PxTransform tmpTr(m_Controller->getPosition().x,
			m_Controller->getPosition().y,
			m_Controller->getPosition().z);

		tmpTr = tmpTr.transform(PxTransform(XMtoPX(
			Vector3::ScalarProduct(m_xmf3Look, 30, false)
		)));
		m_AttackDamage->randDamage();
		//*reinterpret_cast<DamageVal*>(m_weaponTrigger->userData) = m_AttackDamage;
		//memcpy(m_weaponTrigger->userData, &m_AttackDamage, sizeof(DamageVal));
		//*(DamageVal*)m_weaponTrigger->userData = m_AttackDamage;
		//*(int*)m_weaponTrigger->userData = m_damage;
		m_weaponTrigger->setGlobalPose(tmpTr, true);
	}
}

void Player::Attack_Normal()
{
	m_AttackDamage->baseDamage = m_damage;
	m_AttackDamage->hitback = 0.0f;
	m_AttackDamage->paticleType = 0;
	Player::Attack();
}

void Player::Attack_Kick()
{
	m_AttackDamage->baseDamage = m_damage * 2;
	m_AttackDamage->hitback = 1.3f;
	m_AttackDamage->paticleType = 1;
	Player::Attack();
}

void Player::Attack_Upper()
{
	m_AttackDamage->baseDamage = m_damage * 3;
	m_AttackDamage->hitback = 1.3f;
	m_AttackDamage->paticleType = 2;
	Player::Attack();
}

void Player::Attack_Power()
{
	m_AttackDamage->baseDamage = m_damage * 5;
	m_AttackDamage->hitback = 0.0f;
	m_AttackDamage->paticleType = 3;
	Player::Attack();
}

void Player::Hitted(int damage)
{
	if (m_avoid)
		return;
	setHitted(true);
	if (m_status->m_health <= 0 || m_status->m_health > 10000) {
		m_status->m_health = 0;
		m_playerLogic->changeState(STATE::death);
		return;
	}
	m_status->m_health -= damage;
#ifdef _DEBUG
	cout << "Player Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;;
#endif
	m_playerLogic->changeState(STATE::hitted);
	SoundMgr::getInstance()->play(SOUND::HURT01, CHANNEL::PLAYER);
	m_pCamera->ShakeInit();
}


void Player::Hitted(DamageVal& hitback)
{
	if (m_avoid)
		return;
	if (m_status->m_health <= 0 || m_status->m_health > 10000) {
		m_status->m_health = 0;
		m_playerLogic->changeState(STATE::death);
		return;
	}
	setHitted(true);
	m_status->m_health -= hitback.baseDamage;
#ifdef _DEBUG
	cout << "Player Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;;
#endif
	m_playerLogic->changeState(STATE::hitted);
	hitBackstep = hitback.hitback;
	SoundMgr::getInstance()->play(SOUND::HURT01, CHANNEL::PLAYER);
	m_pCamera->ShakeInit();
}

void Player::Skill()
{
	if(!m_avoid)
		m_avoid = true;
}

void Player::SetPosition(float x, float y, float z)
{
	//플레이어 강제 이동 함수 (텔레포트)
	//방에서 방 이동시 호출
	m_Controller->setPosition(PxExtendedVec3(x, y, z));
	//m_moveSpeed = 0;
	if (m_pCamera) {
		//카메라 회전값 리셋
		m_pCamera->ResetRotation();
		if (x > z) {
			if(x > 0)
				m_pCamera->Rotate(0, 270, 0);
			else 
				m_pCamera->Rotate(0, 0, 0);
		}
		else {
			if(z > 0)
				m_pCamera->Rotate(0, 180, 0);
			else
				m_pCamera->Rotate(0, 90, 0);
		}
		//카메라를 플레이어 위치로 강제이동 (물리 작용X, 텔레포트)
		m_cameraController->setPosition(m_Controller->getPosition());
		m_pCamera->SetPosition(PXtoXM(m_cameraController->getPosition()));
		m_pCamera->RegenerateViewMatrix();

	}
}

void Player::Animate(float fTime)
{
	m_weaponTrigger->setGlobalPose(PxTransform(100, 100, 100), false); //공격 트리거 박스 초기화
	m_avoid = false;
	m_playerLogic->update(fTime); //상태머신 수행
	if (m_playerLogic->getState() != STATE::hitted)
		hitBackstep = 0.0f;
	if (m_status->m_health <= 0 || m_status->m_health > 10000) {
		m_status->m_health = 0;
		m_playerLogic->changeState(STATE::death);
	}
	ModelObject::Animate(fTime); //애니메이션
	
	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(fTime), 0), 0.1f, fTime, m_ControllerFilter);

		//hitted 뒤로 밀림 적용
		XMFLOAT3 hitback = Vector3::Add(XMFLOAT3(0, 0, 0), GetLookVector(), -hitBackstep);

		m_Controller->move(XMtoPX(hitback), 0.1f, fTime, m_ControllerFilter);
		m_xmf3Position = PXtoXM(m_Controller->getFootPosition()); //발 좌표로 이동 보정
		RegenerateMatrix(); //이동 회전을 매트릭스에 적용
		
	}

	if (m_pCamera) {
		XMFLOAT3 nowPos = GetPosition();
		m_pCamera->Update(nowPos, fTime);
		//Update함수에서 카메라 이동백터를 nowPos에 반환, move함수에서 실제 카메라 이동
		m_cameraController->move(XMtoPX(nowPos), 0.01f, fTime, m_ControllerFilter);
		//physX에서 연산한 이동값을 실제 카메라에 대입 후 Matrix 재생성
		m_pCamera->SetPosition(PXtoXM(m_cameraController->getPosition()));
		m_pCamera->RegenerateViewMatrix();
	}

	if (!m_Jump.mJump)
		m_Jump.startJump(PxF32(0)); //중력 작용

	
}

void Player::SetCamera(Camera * tCamera, BasePhysX* phys)
{
	m_pCamera = tCamera;
	m_pCamera->SetPlayer(this);

	m_cameraController = phys->getBoxController(XMtoPXEx(m_pCamera->GetPosition()), &m_CameraCallback, XMFLOAT3(5.0f, 5.0f, 5.0f), 30.0f, 10.0f);

	m_weaponTrigger = phys->getTrigger(PxVec3(100,100,100), XMFLOAT3(10,10,10));
	m_weaponTrigger->userData = m_AttackDamage;
}

void Player::CalibrateLook(XMFLOAT3& look)
{
	//플레이어가 항상 카메라의 룩벡터만 바라보도록 보정
	m_xmf3Look = look;
	m_xmf3Look.y = 0.0f; //y축은 무시
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void Player::reset()
{
	//m_status->m_health = m_status->prev_health;
	m_status->reset();
	m_playerLogic->reset();
	SetLive(true);
	ChangeAnimation(PlayerAni::Idle);
	stopAnim(false);

	m_skillCostBouns = 0;
	m_mpCostBouns = 0;
	m_damage = 10;
}

void Player::roomClearBouns(ClearBouns index)
{
#ifdef _DEBUG
	cout << "clear bouns number : " << index << endl;
#endif
	switch (index) {
	case ClearBouns::mpCost:
		m_mpCostBouns = 1;
		break;
	case ClearBouns::skillCost:
		m_skillCostBouns = 1000;
		break;
	case ClearBouns::plusAtk:
		m_damage += 2;
		break;
	case ClearBouns::plusHP:
		m_status->m_maxhealth += 30;
		break;
	case ClearBouns::plusMP:
		m_status->m_maxmp += 1;
		break;
	case ClearBouns::recoveryHP:
		m_status->m_health += 50;
		if (m_status->m_health > m_status->m_maxhealth)
			m_status->m_health = m_status->m_maxhealth;
		break;
	case ClearBouns::recoveryMP:
		m_status->m_mp += 3;
		if (m_status->m_mp > m_status->m_maxmp)
			m_status->m_mp = m_status->m_maxmp;
		break;
	default:
#ifdef _DEBUG
		cout << "undefined bouns" << endl;
#endif
		break;
	}
}

UINT Player::roomClearBouns()
{
	UINT index = rand() % 7;
#ifdef _DEBUG
	cout << "clear bouns number : " << index << endl;
#endif

	switch (index) {
	case ClearBouns::mpCost:
		m_mpCostBouns = 1;
		break;
	case ClearBouns::skillCost:
		m_skillCostBouns = 1000;
		break;
	case ClearBouns::plusAtk:
		m_damage += 2;
		break;
	case ClearBouns::plusHP:
		m_status->m_maxhealth += 30;
		break;
	case ClearBouns::plusMP:
		m_status->m_maxmp += 1;
		break;
	case ClearBouns::recoveryHP:
		m_status->m_health += 50;
		if (m_status->m_health > m_status->m_maxhealth)
			m_status->m_health = m_status->m_maxhealth;
		break;
	case ClearBouns::recoveryMP:
		m_status->m_mp += 3;
		if (m_status->m_mp > m_status->m_maxmp)
			m_status->m_mp = m_status->m_maxmp;
		break;
	default:
#ifdef _DEBUG
		cout << "undefined bouns" << endl;
#endif
		break;
	}

	return index;
}
