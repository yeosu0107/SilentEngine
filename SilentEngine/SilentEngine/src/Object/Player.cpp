#include "stdafx.h"
#include "Player.h"

Player::Player(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
	: ModelObject(model, pd3dDevice, pd3dCommandList)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pCamera = nullptr;

	m_Jump.startJump(PxF32(0)); //중력 작용을 위해 점프 임시로 점프 세팅
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
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}
	if (m_pCamera) {
		//m_pCamera->Rotate(x, y, z);
		m_pCamera->Rotate(0, y, 0); //y축 회전만 허용
	}
}

void Player::RegenerateMatrix()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._12= m_xmf3Right.y; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._13 = m_xmf3Right.z; m_xmf4x4World._23 = m_xmf3Up.z; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
	GameObject::Rotate(0, 180, 0); //정면 쳐다보기
}

void Player::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	m_ani[PlayerAni::Idle]->SetAnimSpeed(1.0f);
	m_ani[PlayerAni::Move]->SetAnimSpeed(1.0f);
	m_ani[PlayerAni::Attack]->SetAnimSpeed(0.5f);
	m_ani[PlayerAni::Idle]->SetAnimSpeed(1.0f);
}

bool Player::Move(DWORD input, float fDist)
{
	if (input) {
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (input & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDist);
		if (input & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDist);
		if (input & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDist);
		if (input & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDist);
		if (input & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDist);
		if (input & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDist);

		if (m_Controller) {
			m_Controller->move(XMtoPX(xmf3Shift), 1.0f, 1, m_ControllerFilter);
			//SetPosition(PXtoXM(m_Controller->getPosition())); //애니메에트에서 처리
		}
		//플레이어가 항상 카메라의 룩벡터만 바라보도록 보정
		m_xmf3Look = m_pCamera->GetLookVector();
		m_xmf3Look.y = 0.0f; //y축은 무시
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
		m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
		m_AnimIndex = PlayerAni::Move;
		return true;
	}
	return false;
}

bool Player::Movement(DWORD input)
{
	m_AnimIndex = PlayerAni::Idle;

	if (input & ANI_ATTACK)
		m_AnimIndex = PlayerAni::Attack;
	if (input & ANI_SKILL)
		m_AnimIndex = PlayerAni::Skill;

	if (input != 0)
		return true;
	return false;
}

void Player::Animate(float fTime)
{
	ModelObject::Animate(fTime); //애니메이션

	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(1.0f/60.0f), 0), 0.1f, 1.0f / 60.0f, m_ControllerFilter);
		//SetPosition(PXtoXM(m_Controller->getPosition()));
		//SetPosition(PXtoXM(m_Controller->getFootPosition())); //발 좌표로 이동 보정
		m_xmf3Position = PXtoXM(m_Controller->getFootPosition());
		RegenerateMatrix(); //이동 회전을 매트릭스에 적용
	}
	if (m_pCamera) {
		m_pCamera->Update(GetPosition(), 1.0f/60.0f);
		m_pCamera->SetLookAt(GetPosition());
		m_pCamera->RegenerateViewMatrix();
	}
}

void Player::SetCamera(Camera * tCamera)
{
	m_pCamera = tCamera;
	m_pCamera->SetPlayer(this);
}