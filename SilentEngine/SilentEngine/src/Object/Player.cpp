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
}

Player::~Player()
{
	if (m_pCamera)
		delete m_pCamera;

	ModelObject::~ModelObject();
}

void Player::Move(DWORD dir, float fDist)
{
	//XMFLOAT3 xmf3PlayerMoveForward = Vector3::SubtractAxisZero(GetPosition(), m_pCamera->GetPosition(), RotY);
	//XMFLOAT3 xmf3PlayerMoveRight = m_pCamera->GetRightVector();

	if (dir) {
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dir & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDist);
		if (dir & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDist);
		if (dir & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDist);
		if (dir & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDist);
		if (dir & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDist);
		if (dir & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDist);

		if (m_Controller) {
			m_Controller->move(XMtoPX(xmf3Shift)*0.1f, 0.001f, 1, m_ControllerFilter);
			SetPosition(PXtoXM(m_Controller->getPosition()));
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
}
