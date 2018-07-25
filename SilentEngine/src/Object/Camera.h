#pragma once

#include "D3DUtil.h"
#include "D3DMath.h"
#include "UploadBuffer.h"

#define FIRST_PERSON_CAMERA 0x01
#define THIRD_PERSON_CAMERA 0x03


class Player;

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;
	XMFLOAT4X4						m_xmf4x4InvProjection;
	XMFLOAT4X4						m_xmf4x4ShadowProjection[NUM_DIRECTION_LIGHTS];
	XMFLOAT3						m_xmf3Position;
};

class Camera
{
public:
	Camera();
	Camera(Camera* pCamera);
	~Camera();

public:
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void InitCamera(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildDescriptorHeaps(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pCommandList, VS_CB_CAMERA_INFO& cbInfo);
	virtual void SetShadowProjection(const XMFLOAT4X4& other, int index) { m_xmf4x4ShadowProjection[index] = other; }
public:
	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
		float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
	

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList);
	float AspectRatio() const;

protected:
	//ī�޶��� ��ġ
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3OriginPos;
	bool			m_isShake = false;


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

	XMFLOAT4X4 m_xmf4x4ShadowProjection[NUM_DIRECTION_LIGHTS];


	//����Ʈ
	D3D12_VIEWPORT m_d3dViewport;
	//���� �簢�� 
	D3D12_RECT m_d3dRect;

	//ī�޶� ���� �÷��̾� ������
	Player *m_pPlayer = NULL;

	const UINT			m_ClientWidth = FRAME_BUFFER_WIDTH;
	const UINT			m_ClientHeight = FRAME_BUFFER_HEIGHT;
	UINT				m_nCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12DescriptorHeap>				m_SrvDescriptorHeap = nullptr;
	ComPtr<ID3D12RootSignature>					m_RootSignature = nullptr;
	unique_ptr<UploadBuffer<VS_CB_CAMERA_INFO>>	m_ObjectCB = nullptr;
	VS_CB_CAMERA_INFO							*m_pcbMappedCamera;

public:
	//ī�޶� ��ȯ��� ����
	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	//ī�޶� ������ ����(���� �����ϵ���)
	void RegenerateViewMatrix();

	//������ȯ��� ����
	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);
	void SetPlayer(Player *pPlayer) { m_pPlayer = pPlayer; }
	Player *GetPlayer() { return m_pPlayer; }

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return(m_nMode); }

	virtual void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }

	UploadBuffer<VS_CB_CAMERA_INFO>* GetUploadBuffer() const { return m_ObjectCB.get(); }
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

	void ResetRotation() { m_xmf4x4Rotate = Matrix4x4::Identity(); }
	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dRect); }

	virtual void Move(const XMFLOAT3& xmf3Shift) {
		m_xmf3Position.x += xmf3Shift.x;
		m_xmf3Position.y += xmf3Shift.y;
		m_xmf3Position.z += xmf3Shift.z;
	}
	virtual void Move(ULONG dwDirection, float fDistance, bool bVelocity = false);

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

	//ī�޶��� �̵�, ȸ���� ���� ī�޶��� ������ �����ϴ� �����Լ��̴�. 
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) {};

	//3��Ī ī�޶󿡼� ī�޶� �÷��̾ �ٶ󺸰� ����
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }

	void ReleaseShaderVariables();

	virtual void ShakeInit() {}
	virtual void Shake(float amount) {}
};

class CThirdPersonCamera : public Camera
{
private:
	const char baseShakeCount = 5;
	char			m_nowShakeCount = 0;

public:
	CThirdPersonCamera();
	CThirdPersonCamera(Camera *pCamera);
	virtual ~CThirdPersonCamera() { };
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
	virtual void SetPosition(XMFLOAT3 xmf3Position);

	virtual void ShakeInit();
	virtual void Shake(float amount);
};