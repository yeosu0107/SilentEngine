#pragma once
#include "Mesh.h"
#include "Camera.h"

class CGameObject
{
private:
protected:
	bool die = false;
	XMFLOAT4 color;

	float width = 0.0f;
	float height = 0.0f;
	float depth = 0.0f;

	XMFLOAT4X4 m_xmf4x4World;
	CMesh **m_ppMeshes = NULL;

	int m_nMeshes = 0;
	int m_nReferences = 0;

	BoundingOrientedBox m_xmOOBB; //모델좌표계에서의 충돌영역
	BoundingOrientedBox	m_xmOOBBTransformed; //월드좌표계에서의 충돌 영역

	XMFLOAT3 movingDir;
public:
	CGameObject(int nMeshes = 1);
	virtual ~CGameObject();

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();
	virtual void SetMesh(int nIndex, CMesh *pMesh);
	//virtual void SetShader(CShader *pShader);
	//virtual void SetObject(float w, float h, float d);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT
		nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView);

	//상수 버퍼를 생성한다. 
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다. 
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Scale(float num);
	void setScale(float num);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void MoveVector(XMFLOAT3 dir, float fDistance = 1.0f);
	void Move(float fDist = 3.0f);

	void setMovingDir(XMFLOAT3 dir) { movingDir = dir; }
	XMFLOAT3& getMovingDir() { return movingDir; }

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
	void SetOOBB(BoundingOrientedBox& oobb) { m_xmOOBB = oobb; }
	BoundingOrientedBox* getOOBB() { return &m_xmOOBBTransformed; } //트랜스폼 oobb주소 반환
	XMFLOAT4X4& getMatrix() { return m_xmf4x4World; }

	bool getDie() const { return die; }
	void setDie(bool type) { die = type; }


	XMFLOAT4& getColor() { return color; }
	void setColor(XMFLOAT4& t) { color = t; }

	//모델 좌표계의 픽킹 광선을 생성한다.
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition,
		XMFLOAT4X4& xmf4x4View, XMFLOAT3 *pxmf3PickRayOrigin, XMFLOAT3 *pxmf3PickRayDirection);
	//카메라 좌표계의 한 점에 대한 모델 좌표계의 픽킹 광선을 생성하고 객체와의 교차를 검사한다. 
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition,
		XMFLOAT4X4& xmf4x4View, float *pfHitDistance);

	bool IsVisible(CCamera *pCamera = NULL);


};

class CRotatingObject : public CGameObject
{
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;

public:
	CRotatingObject(int nMeshes = 1);
	virtual ~CRotatingObject();

	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }
	virtual void Animate(float fTimeElapsed);

};