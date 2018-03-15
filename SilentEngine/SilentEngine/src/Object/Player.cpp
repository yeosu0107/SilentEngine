#include "stdafx.h"
#include "Player.h"


Player::Player(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
}


Player::~Player()
{
}


void Player::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
}

void Player::ReleaseShaderVariables()
{
}

void Player::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
}

void Player::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	// 카메라와 플레이어의 x ,z 차이에 대한 값만 갖고 있는 벡터를 생성
	XMFLOAT3 xmf3PlayerMoveForward = Vector3::SubtractAxisZero(m_xmf3Position, m_pCamera->GetPosition(), RotY);
	XMFLOAT3 xmf3PlayerMoveRight = m_pCamera->GetRightVector();

	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, xmf3PlayerMoveForward, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, xmf3PlayerMoveForward, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, xmf3PlayerMoveRight, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, xmf3PlayerMoveRight, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);


		if (Vector3::AngleAxisZero(m_xmf3Look, xmf3Shift, RotY) > 5.0f)
		{
			m_bIsLotate = true;
			m_xmf3LookDist = xmf3Shift;
		}

		Move(m_xmf3Look, bUpdateVelocity);
	}
}

void Player::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void Player::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	
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
	//m_pCamera->Rotate(x, y, z);
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void Player::Update(float fTimeElapsed)
{
	if (m_bIsLotate)
		RotateLookAt(fTimeElapsed);

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x +
		m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;

	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}

	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);

	if (fLength > m_fMaxVelocityY)
		m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	Move(m_xmf3Velocity, false);

	if (m_pPlayerUpdatedContext)
		OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();

	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	if (m_pCameraUpdatedContext)
		OnCameraUpdateCallback(fTimeElapsed);

	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

Camera *Player::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	Camera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void Player::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera)
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void Player::SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->Resource()->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_PLAYER, d3dGpuVirtualAddress);
}

void Player::Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) GameObject::Render(pd3dCommandList, pCamera);
}


void Player::RotateLookAt(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerLook;
	XMFLOAT3 xmf3DistVector = m_xmf3LookDist;

	float fAngle = Vector3::Angle(Vector3::Normalize(m_xmf3Look), Vector3::Normalize(xmf3DistVector));

	if (fAngle - 180.0f <= FLT_EPSILON && fAngle - 180.0f >= -FLT_EPSILON) {
		XMFLOAT3 xmf3CrossLookUp = Vector3::CrossProduct(m_xmf3Look, m_xmf3Up);
		xmf3PlayerLook = Vector3::Add(m_xmf3Look, Vector3::Lerp(m_xmf3Look, xmf3CrossLookUp, fTimeElapsed * 1.0f));
	}
	else if (fAngle > 5.0f) {
		xmf3PlayerLook = Vector3::Add(m_xmf3Look, Vector3::Lerp(m_xmf3Look, xmf3DistVector, fTimeElapsed * 1.0f));
	}
	else {
		xmf3PlayerLook = xmf3DistVector;
		m_bIsLotate = false;
	}

	fAngle = Vector3::AngleAxisZero(m_xmf3Look, xmf3PlayerLook, RotY);
	fAngle *= Vector3::TripleProduct(m_xmf3Look, m_xmf3Up, m_xmf3LookDist) >= 0 ? -1 : 1;

	Rotate(0.0f, fAngle, 0.0f);

}

/////////////////////////////////////////////

MyPlayer::MyPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes) : Player(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CCubeMeshIlluminatedTextured *pCubeMesh = new CCubeMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 4.0f);
	SetMesh(0, pCubeMesh);

	UINT ncbElementBytes = ((sizeof(CB_PLAYER_INFO) + 255) & ~255); //256의 배수

	//PlayerShader *pShader = new CPlayerShader();
	//Shader->CreateGraphicsRootSignature(pd3dDevice);
	//Shader->CreateShader(pd3dDevice, pShader->GetGraphicsRootSignature());
	//Shader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//Shader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 0);
	//Shader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbPlayer, ncbElementBytes);
	//
	//etCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());
	//
	//etShader(pShader);
}

MyPlayer::~MyPlayer()
{

}

Camera *MyPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//3인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다. 
		//SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 30.0f, -60.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 500.0f, ASPECT_RATIO, 60.0f);
		break;
	default:
		break;
	}
	m_pCamera->SetOOBB(m_pCamera->GetPosition(), XMFLOAT3(4, 7, 7), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	Update(fTimeElapsed);
	return(m_pCamera);
}

