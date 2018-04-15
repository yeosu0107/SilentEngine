#include "stdafx.h"
#include "Player.h"

const float startSpeed = 30.0f;
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
		m_pCamera->Rotate(x, y, z);
		//m_pCamera->Rotate(0, y, 0); //y�� ȸ���� ���
	}
}

void Player::RegenerateMatrix()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._12= m_xmf3Right.y; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._13 = m_xmf3Right.z; m_xmf4x4World._23 = m_xmf3Up.z; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;

	//������ 0.5��
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMLoadFloat4x4(&m_xmf4x4World));
	GameObject::Rotate(0, 180, 0); //���� �Ĵٺ���
	
}

void Player::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	//m_ani[PlayerAni::Idle]->SetAnimSpeed(1.0f);
	//m_ani[PlayerAni::Move]->SetAnimSpeed(0.5f);
	//m_ani[PlayerAni::Attack]->SetAnimSpeed(0.5f);
	//m_ani[PlayerAni::Idle]->SetAnimSpeed(1.0f);
	m_ani[PlayerAni::Idle]->EnableLoof();
}

bool Player::Move(DWORD input, float fTime)
{
	if (input){
		float fDist = m_moveSpeed * fTime;
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		XMFLOAT3 tmpLook=XMFLOAT3(0,0,0);

		//ȸ�� ���
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

		//�÷��̾� �躤�� ����
		if (!Vector3::IsZero(tmpLook)) {
			tmpLook = Vector3::Normalize(tmpLook);
			CalibrateLook(tmpLook);
		}
		else
			return false; //�躤�Ͱ� 0�ΰ��� �̵�X, false ����

		//�̵� �Ÿ� ���
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDist);

		//���� ó��
		if (m_moveSpeed < maxSpeed)
			m_moveSpeed += accelSpeed;
		
		if (m_Controller) {
			m_Controller->move(XMtoPX(xmf3Shift), 0.001f, 1, m_ControllerFilter);
			//���� ���� ������Ʈ�� �̵��� �ִϸ޿�Ʈ���� ó�� (����� ������ �̵��� ó��)
		}
		m_AnimIndex = PlayerAni::Move;
		//ChangeAnimation(PlayerAni::Move);
		return true;
	}
	m_moveSpeed = startSpeed;
	return false;
}

bool Player::Movement(DWORD input)
{
	m_AnimIndex = PlayerAni::Idle;
	//ChangeAnimation(PlayerAni::Idle);
	
	if (input & ANI_ATTACK) {
		//m_AnimIndex = PlayerAni::Attack;
		Attack();
	}
	if (input & ANI_SKILL)
		m_AnimIndex = PlayerAni::Skill;

	if (input != 0)
		return true;
	return false;
}

void Player::Attack()
{
	PxTransform tmpTr(m_Controller->getPosition().x,
		m_Controller->getPosition().y,
		m_Controller->getPosition().z);

	tmpTr = tmpTr.transform(PxTransform(XMtoPX(
		Vector3::ScalarProduct(m_xmf3Look, 30, false)
	)));

	m_weaponTrigger->setGlobalPose(tmpTr, true);
}

void Player::SetPosition(float x, float y, float z)
{
	//�÷��̾� ���� �̵� �Լ� (�ڷ���Ʈ)
	//�濡�� �� �̵��� ȣ��
	m_Controller->setPosition(PxExtendedVec3(x, y, z));
	m_moveSpeed = 0;
	if (m_pCamera) {
		//ī�޶� ȸ���� ����
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
		//ī�޶� �÷��̾� ��ġ�� �����̵� (���� �ۿ�X, �ڷ���Ʈ)
		m_cameraController->setPosition(m_Controller->getPosition());
		m_pCamera->SetPosition(PXtoXM(m_cameraController->getPosition()));
		m_pCamera->RegenerateViewMatrix();

	}
}

void Player::Animate(float fTime)
{
	ModelObject::Animate(fTime); //�ִϸ��̼�
	
	if (m_Controller) {
		//�߷��ۿ� ó��
		m_Controller->move(PxVec3(0, m_Jump.getHeight(fTime), 0), 0.1f, fTime, m_ControllerFilter);
		m_xmf3Position = PXtoXM(m_Controller->getFootPosition()); //�� ��ǥ�� �̵� ����
		RegenerateMatrix(); //�̵� ȸ���� ��Ʈ������ ����
		m_weaponTrigger->setGlobalPose(PxTransform(100, 100, 100), false);
	}

	if (m_pCamera) {
		XMFLOAT3 nowPos = GetPosition();
		m_pCamera->Update(nowPos, fTime);
		//Update�Լ����� ī�޶� �̵����͸� nowPos�� ��ȯ, move�Լ����� ���� ī�޶� �̵�
		m_cameraController->move(XMtoPX(nowPos), 0.01f, fTime, m_ControllerFilter);
		//physX���� ������ �̵����� ���� ī�޶� ���� �� Matrix �����
		m_pCamera->SetPosition(PXtoXM(m_cameraController->getPosition()));
		m_pCamera->RegenerateViewMatrix();
	}

	if (!m_Jump.mJump)
		m_Jump.startJump(PxF32(0)); //�߷� �ۿ�
}

void Player::SetCamera(Camera * tCamera, BasePhysX* phys)
{
	m_pCamera = tCamera;
	m_pCamera->SetPlayer(this);
	string* tmp = new string("camera");
	m_cameraController = phys->getBoxController(XMtoPXEx(m_pCamera->GetPosition()), &m_CameraCallback, tmp, 30.0f, 10.0f);

	m_weaponTrigger = phys->getTrigger(PxVec3(100,100,100));
}

void Player::CalibrateLook(XMFLOAT3& look)
{
	//�÷��̾ �׻� ī�޶��� �躤�͸� �ٶ󺸵��� ����
	m_xmf3Look = look;
	m_xmf3Look.y = 0.0f; //y���� ����
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}
