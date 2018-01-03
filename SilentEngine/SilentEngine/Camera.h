#pragma once

//��Ⱦ��
#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

//ī�޶� ���
#define FIRST_PERSON_CAMERA 0x01

#define THIRD_PERSON_CAMERA 0x03

class CPlayer;

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;
	XMFLOAT3						m_xmf3Position;
};

class CCamera
{
protected:
	//ī�޶��� ��ġ
	XMFLOAT3 m_xmf3Position;

	//ī�޶��� ���� x-��(Right), y-��(Up), z-��(Look)
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//ȸ������ 
	float m_fPitch;
	float m_fRoll;
	float m_fYaw;

	//ī�޶��� ����(1��Ī ī�޶�, �����̽�-�� ī�޶�, 3��Ī ī�޶�)
	DWORD m_nMode;
	//�÷��̾ �ٶ� ��ġ �����̴�. �ַ� 3��Ī ī�޶󿡼� ���ȴ�.
	XMFLOAT3 m_xmf3LookAtWorld;
	//�÷��̾�� ī�޶��� �������� ��Ÿ���� �����̴�. �ַ� 3��Ī ī�޶󿡼� ���ȴ�.
	XMFLOAT3 m_xmf3Offset;

	//ȸ�������ð�
	float m_fTimeLag;

	//ī�޶� ��ȯ ��� 
	XMFLOAT4X4 m_xmf4x4View;
	//���� ��ȯ ��� 
	XMFLOAT4X4 m_xmf4x4Projection;
	// ī�޶� ȸ�� ���
	XMFLOAT4X4 m_xmf4x4Rotate;

	//����Ʈ
	D3D12_VIEWPORT m_d3dViewport;
	//���� �簢�� 
	D3D12_RECT m_d3dScissorRect;

	//ī�޶� ���� �÷��̾� ������
	CPlayer *m_pPlayer = NULL;

	//ī�޶� oobb
	BoundingOrientedBox m_xmOOBB;			 //����ǥ�迡���� �浹����
	BoundingOrientedBox	m_xmOOBBTransformed; //������ǥ�迡���� �浹 ����
	bool				CameraCrush;
	BoundingFrustum     m_xmFrustum;			 //����ü �ø�
	ID3D12Resource					*m_pd3dcbCamera = NULL;
	VS_CB_CAMERA_INFO				*m_pcbMappedCamera = NULL;
public:
	CCamera();
	CCamera(CCamera* pCamera);
	virtual ~CCamera();

	//ī�޶����� -> ���̴� ���α׷�
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice,
		ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	//ī�޶� ��ȯ��� ����
	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	//ī�޶� ������ ����(���� �����ϵ���)
	void RegenerateViewMatrix();


	//������ȯ��� ����
	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	//����Ʈ, ������Ʈ ��
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

	virtual void Move(const XMFLOAT3& xmf3Shift) {
		m_xmf3Position.x += xmf3Shift.x;
		m_xmf3Position.y += xmf3Shift.y;
		m_xmf3Position.z += xmf3Shift.z;
	}

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }

	//ī�޶��� �̵�, ȸ���� ���� ī�޶��� ������ �����ϴ� �����Լ��̴�. 
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) { }

	//3��Ī ī�޶󿡼� ī�޶� �ٶ󺸴� ������ �����Ѵ�. 
	//�Ϲ������� �÷��̾ �ٶ󺸵��� �����Ѵ�. 
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }

	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation) { m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation); }
	BoundingOrientedBox* getOOBB() { return &m_xmOOBBTransformed; } //Ʈ������ oobb�ּ� ��ȯ
	void UpdateOOBB(XMFLOAT4X4& matrix);
	void setCrush(bool t) { CameraCrush = t; }
	bool getCrush() const { return CameraCrush; }
	bool IsInFrustum(BoundingOrientedBox& xmBoundingBox);
	//����ü(���� ��ǥ��)�� �����Ѵ�. 
	void GenerateFrustum();
	//�ٿ�� �ڽ�(OOBB, ���� ��ǥ��)�� ����ü�� ���ԵǴ� ���� �˻��Ѵ�. 


};

class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { };
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

public:
	bool RotateLock(XMFLOAT3& xmf3Direction, XMFLOAT3& xmf3CameraPos);

};