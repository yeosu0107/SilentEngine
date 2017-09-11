#pragma once

//종횡비
#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

//카메라 모드
#define FIRST_PERSON_CAMERA 0x01

#define THIRD_PERSON_CAMERA 0x03

class CPlayer;

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Projection;
};

class CCamera
{
protected:
	//카메라의 위치
	XMFLOAT3 m_xmf3Position;

	//카메라의 로컬 x-축(Right), y-축(Up), z-축(Look)
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//회전각도 
	float m_fPitch;
	float m_fRoll;
	float m_fYaw;

	//카메라의 종류(1인칭 카메라, 스페이스-쉽 카메라, 3인칭 카메라)
	DWORD m_nMode;
	//플레이어가 바라볼 위치 벡터이다. 주로 3인칭 카메라에서 사용된다.
	XMFLOAT3 m_xmf3LookAtWorld;
	//플레이어와 카메라의 오프셋을 나타내는 벡터이다. 주로 3인칭 카메라에서 사용된다.
	XMFLOAT3 m_xmf3Offset;

	//회전지연시간
	float m_fTimeLag;

	//카메라 변환 행렬 
	XMFLOAT4X4 m_xmf4x4View;
	//투영 변환 행렬 
	XMFLOAT4X4 m_xmf4x4Projection;
	//뷰포트
	D3D12_VIEWPORT m_d3dViewport;
	//씨저 사각형 
	D3D12_RECT m_d3dScissorRect;

	//카메라 소유 플레이어 포인터
	CPlayer *m_pPlayer = NULL;

	//카메라 oobb
	BoundingOrientedBox m_xmOOBB;			 //모델좌표계에서의 충돌영역
	BoundingOrientedBox	m_xmOOBBTransformed; //월드좌표계에서의 충돌 영역
	bool				CameraCrush;
	BoundingFrustum     m_xmFrustum;			 //절두체 컬링


public:
	CCamera();
	CCamera(CCamera* pCamera);
	virtual ~CCamera();

	//카메라정보 -> 셰이더 프로그램
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice,
		ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	//카메라 변환행렬 생성
	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	//카메라 로컬축 보정(서로 직교하도록)
	void RegenerateViewMatrix();


	//투영변환행렬 생성
	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	//뷰포트, 시저렉트 셋
	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
		float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return m_pPlayer; }

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return(m_nMode); }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }

	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) {
		m_xmf3LookAtWorld =
			xmf3LookAtWorld;
	}
	XMFLOAT3& GetLookAtPosition() { return(m_xmf3LookAtWorld); }

	XMFLOAT3& GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	void SetOffset(XMFLOAT3 xmf3Offset) {
		m_xmf3Offset = xmf3Offset;
		m_xmf3Position.x += xmf3Offset.x;
		m_xmf3Position.y += xmf3Offset.y;
		m_xmf3Position.z += xmf3Offset.z;
	}
	XMFLOAT3& GetOffset() { return(m_xmf3Offset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dScissorRect); }

	virtual void Move(XMFLOAT3& xmf3Shift) {
		m_xmf3Position.x += xmf3Shift.x;
		m_xmf3Position.y += xmf3Shift.y;
		m_xmf3Position.z += xmf3Shift.z;
	}

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }

	//카메라의 이동, 회전에 따라 카메라의 정보를 갱신하는 가상함수이다. 
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) { }

	//3인칭 카메라에서 카메라가 바라보는 지점을 설정한다. 
	//일반적으로 플레이어를 바라보도록 설정한다. 
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
	BoundingOrientedBox* getOOBB() { return &m_xmOOBBTransformed; } //트랜스폼 oobb주소 반환
	void UpdateOOBB(XMFLOAT4X4& matrix);
	void setCrush(bool t) { CameraCrush = t; }
	bool getCrush() const { return CameraCrush; }
	bool IsInFrustum(BoundingOrientedBox& xmBoundingBox);
	//절두체(월드 좌표계)를 생성한다. 
	void GenerateFrustum();
	//바운딩 박스(OOBB, 월드 좌표계)가 절두체에 포함되는 가를 검사한다. 


};

//1인칭 시점
class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera *pCamera);
	virtual ~CFirstPersonCamera() { }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

//3인칭 시점
class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { }
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};