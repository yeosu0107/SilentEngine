#pragma once
#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
protected:

	CCamera *m_pCamera = nullptr;

	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	XMFLOAT3 m_xmf3LookDist;

	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	//float width = 20.0f;
	//float height = 20.0f;
	//float depth = 4.0f;

	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmf3Gravity;

	float m_fMaxVelocityXZ;
	float m_fMaxVelocityY;

	//������
	float m_fFriction;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3 GetLookDist() const { return(m_xmf3LookDist); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetLookDist(XMFLOAT3& xmf3LookDist) { m_xmf3LookDist = xmf3LookDist; }

	void SetPosition(XMFLOAT3& xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x -
			m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),
			false);
	}

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() { return(m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }
	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }


	void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f) {}
	void Move(XMFLOAT3& xmf3Shift, bool bVelocity = false);

	void Rotate(float x, float y, float z);
	void RotateLookAt(float fTimeElapsed);	// �ش� ���͸� �ٶ󺸰� ȸ���ϴ� �Լ�

	void Update(float fTimeElapsed);

	//�÷��̾� ��ġ ����
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	

	//ī�޶� ��ġ ����
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	virtual void OnCameraWallcrush(float fTimeElapsed) {}

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);


	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(nullptr);
	}

	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	virtual void OnPrepareRender();
	//�÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾�(�޽�)�� �������Ѵ�.
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = nullptr);
	virtual void Animate(float fTime);

};

class CMyPlayer : public CPlayer
{
public:
	CMyPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMyPlayer();
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	//virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	//virtual void OnCameraUpdateCallback(float fTimeElapsed);
};