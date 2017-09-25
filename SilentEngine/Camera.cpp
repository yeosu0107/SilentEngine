﻿#include "stdafx.h"
#include "Player.h"
#include "Camera.h"
#include "ConsoleUtily.h"
#include <math.h>

CCamera::CCamera()
{
	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Projection = Matrix4x4::Identity();
	m_xmf4x4Rotate = Matrix4x4::Identity();

	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };

	m_xmf3Position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fTimeLag = 0.0f;
	m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);

	GenerateProjectionMatrix(1.0f, 500.0f, ASPECT_RATIO, 90.0f);
	GenerateViewMatrix(m_xmf3Position, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
}

CCamera::CCamera(CCamera *pCamera)
{
	if (pCamera)
		*this = *pCamera;
	else {
		m_xmf4x4View = Matrix4x4::Identity();
		m_xmf4x4Projection = Matrix4x4::Identity();
		m_xmf4x4Rotate = Matrix4x4::Identity();
		m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
		m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH , FRAME_BUFFER_HEIGHT };
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = 0.0f;
		m_xmf3Offset = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_fTimeLag = 0.0f;
		m_xmf3LookAtWorld = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode = 0x00;
		m_pPlayer = NULL;
	}
}

CCamera::~CCamera()
{
}

void CCamera::CreateShaderVariables(ID3D12Device *pd3dDevice,
	ID3D12GraphicsCommandList *pd3dCommandList) {
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {
	XMFLOAT4X4 xmf4x4View;
	XMStoreFloat4x4(&xmf4x4View, 
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4View, 0);

	XMFLOAT4X4 xmf4x4Projection;
	XMStoreFloat4x4(&xmf4x4Projection,
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4Projection, 16);
}
void CCamera::ReleaseShaderVariables() {
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance,
	float fAspectRatio, float fFOVAngle) {

	m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(fFOVAngle),
		fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
}

void CCamera::GenerateViewMatrix() {
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, m_xmf3LookAtWorld, m_xmf3Up);
}

void CCamera::GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt,
	XMFLOAT3 xmf3Up) {
	m_xmf3Position = xmf3Position;
	m_xmf3LookAtWorld = xmf3LookAt;
	m_xmf3Up = xmf3Up;

	GenerateViewMatrix();
}

void CCamera::RegenerateViewMatrix()
{
	//카메라의 z-축을 기준으로 카메라의 좌표축들이 직교하도록 보정
	//카메라의 z-축 벡터를 정규화한다.
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	//카메라의 z-축과 y-축에 수직인 벡터를 x-축으로 설정한다.
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	//카메라의 z-축과 x-축에 수직인 벡터를 y-축으로 설정한다.
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x;
	m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y;
	m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z;

	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
	GenerateFrustum();
	//printf("%f, %f,  %f\n", m_xmf4x4View._41, m_xmf4x4View._42, m_xmf4x4View._43);
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
	float fMinZ, float fMaxZ) {
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width = float(nWidth);
	m_d3dViewport.Height = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom) {
	m_d3dScissorRect.left = xLeft;
	m_d3dScissorRect.top = yTop;
	m_d3dScissorRect.right = xRight;
	m_d3dScissorRect.bottom = yBottom;
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList) {
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::UpdateOOBB(XMFLOAT4X4& matrix)
{
	m_xmOOBBTransformed = m_xmOOBB;
	m_xmOOBBTransformed.Transform(m_xmOOBBTransformed, XMLoadFloat4x4(&matrix));
	XMStoreFloat4(&m_xmOOBBTransformed.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBBTransformed.Orientation)));
}
void CCamera::GenerateFrustum()
{
	//원근 투영 변환 행렬에서 절두체를 생성한다(절두체는 카메라 좌표계로 표현된다).
	m_xmFrustum.CreateFromMatrix(m_xmFrustum, XMLoadFloat4x4(&m_xmf4x4Projection));
	//카메라 변환 행렬의 역행렬을 구한다. 
	XMMATRIX xmmtxInversView = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4View));
	//절두체를 카메라 변환 행렬의 역행렬로 변환한다(이제 절두체는 월드 좌표계로 표현된다).
	m_xmFrustum.Transform(m_xmFrustum, xmmtxInversView);
}

bool CCamera::IsInFrustum(BoundingOrientedBox& xmBoundingBox)
{
	return(m_xmFrustum.Intersects(xmBoundingBox));
}

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;

}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{

	
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(x));

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up),
			XMConvertToRadians(y));

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look),
			XMConvertToRadians(z));

		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}


	
}


CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	//m_xmf3Offset = XMFLOAT3(0.0f, 50.0f, -50.0f);
}

void CThirdPersonCamera::Rotate(float x, float y, float z) {

	XMFLOAT4X4	xmf4x4Rotate = Matrix4x4::Identity();

	x = min(x, 30.0f);

	if (x != 0.0f)
	{
		
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
			XMConvertToRadians(x));

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

		xmf4x4Rotate = Matrix4x4::Multiply(xmf4x4Rotate, xmmtxRotate);
	}

	if (m_pPlayer && (y != 0.0f))
	{
		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up),
			XMConvertToRadians(y));

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

		xmf4x4Rotate = Matrix4x4::Multiply(xmf4x4Rotate, xmmtxRotate);
	}

	if (m_pPlayer && (z != 0.0f))
	{
		XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Look),
			XMConvertToRadians(z));

		m_xmf3Position = Vector3::Subtract(m_xmf3Position, m_pPlayer->GetPosition());
		m_xmf3Position = Vector3::TransformCoord(m_xmf3Position, xmmtxRotate);
		m_xmf3Position = Vector3::Add(m_xmf3Position, m_pPlayer->GetPosition());

		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);

		xmf4x4Rotate = Matrix4x4::Multiply(xmf4x4Rotate, xmmtxRotate);
	}

	//카메라 오프셋 벡터를 회전 행렬로 변환(회전)한다. 
	XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, Matrix4x4::Multiply(m_xmf4x4Rotate, xmf4x4Rotate));
	//회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것이다.
	XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
	XMFLOAT3 xmf3Direction = Vector3::Subtract(m_pPlayer->GetPosition(), xmf3Position);

	if (RotateLock(xmf3Direction, xmf3Position))
	{
		m_xmf4x4Rotate = Matrix4x4::Multiply(m_xmf4x4Rotate, xmf4x4Rotate);
	}

	//m_xmf4x4Rotate = Matrix4x4::Multiply(m_xmf4x4Rotate, xmf4x4Rotate);
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	if (m_pPlayer)
	{
		XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
		//XMFLOAT3 xmf3Right = GetRightVector();
		//XMFLOAT3 xmf3Up = GetUpVector();
		//XMFLOAT3 xmf3Look = GetLookVector();

		////플레이어의 로컬 x-축, y-축, z-축 벡터로부터 회전 행렬(플레이어와 같은 방향을 나타내는 행렬)을 생성한다.
		//xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
		//xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
		//xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

		//카메라 오프셋 벡터를 회전 행렬로 변환(회전)한다. 
		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, m_xmf4x4Rotate);

		//회전한 카메라의 위치는 플레이어의 위치에 회전한 카메라 오프셋 벡터를 더한 것이다.
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
	
		//방향벡터
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);
		float fLength = Vector3::Length(xmf3Direction);
		xmf3Direction = Vector3::Normalize(xmf3Direction);

		float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
		float fDistance = fLength * fTimeLagScale;

		if (fDistance > fLength) fDistance = fLength;
		if (fLength < 0.01f) fDistance = fLength;
		if (fDistance > 0)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
			SetLookAt(xmf3LookAt);
		}
	}
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer->GetUpVector());

	m_xmf3Right = XMFLOAT3(mtxLookAt._11, mtxLookAt._21, mtxLookAt._31);
	m_xmf3Up = XMFLOAT3(mtxLookAt._12, mtxLookAt._22, mtxLookAt._32);
	m_xmf3Look = XMFLOAT3(mtxLookAt._13, mtxLookAt._23, mtxLookAt._33);
}

bool CThirdPersonCamera::RotateLock(XMFLOAT3& xmf3Direction, XMFLOAT3& xmf3CameraPos)
{
	XMFLOAT3 xmf3tmpCamerapos = xmf3CameraPos;
	XMFLOAT3 xmf3Playerpos = m_pPlayer->GetPosition();
	XMFLOAT3 xmf3ToCameraNorm = Vector3::Normalize(Vector3::ScalarProduct(xmf3Direction, -1));
	// 카메라와 플레이어의 x ,z 차이에 대한 값만 갖고 있는 벡터를 생성
	XMFLOAT3 xmf3ToCameraY0 = Vector3::SubtractYZero(xmf3tmpCamerapos, xmf3Playerpos);

	float fLimitAngle = 60.0f;

	// xmf3ToCameraY0와 xmf3ToCameraNorm의 차이는 사실상 y값밖에 없다. 정규화된 이 두개의 값을 이용하여 값을 추출
	float fAngle = Vector3::Angle(xmf3ToCameraY0, xmf3ToCameraNorm);

	return fAngle < fLimitAngle ? true : false;

}