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

	m_Jump.startJump(PxF32(0));
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
	m_pCamera->Rotate(x, y, z);
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
			XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void Player::Move(DWORD dir, float fDist)
{
	if (dir) {
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dir & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDist);
		if (dir & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDist);
		if (dir & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDist);
		if (dir & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDist);
		if (dir & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDist);
		if (dir & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDist);

		if (m_Controller) {
			m_Controller->move(XMtoPX(xmf3Shift), 1.0f, 1, m_ControllerFilter);
			//SetPosition(PXtoXM(m_Controller->getPosition())); //애니메에트에서 처리
		}
		
	}
}

void Player::Animate(float fTime)
{
	if (m_ani) {
		m_AnimIndex = 0;
		m_ani[m_AnimIndex]->BoneTransform(m_AnimIndex, m_Bones);
		//m_Animtime += 0.03f;
	}
	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(1.0f/60.0f), 0), 0.1f, 1.0f / 60.0f, m_ControllerFilter);
		//SetPosition(PXtoXM(m_Controller->getPosition()));
		SetPosition(PXtoXM(m_Controller->getFootPosition())); //발 좌표로 이동 보정
	}
	if (m_pCamera) {
		m_pCamera->Update(GetPosition(), 1.0f);
		m_pCamera->SetLookAt(GetPosition());
		m_pCamera->RegenerateViewMatrix();
	}
}

void Player::SetCamera(Camera * tCamera)
{
	m_pCamera = tCamera;
	m_pCamera->SetPlayer(this);
}
