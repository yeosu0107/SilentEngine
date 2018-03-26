#pragma once

#include "D3DUtil.h"
#include "..\Model\ModelObject.h"

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

#define FIRST_PERSON_CAMERA 0x01
#define THIRD_PERSON_CAMERA 0x03

#define ROOT_PARAMETER_CAMERA		0
#define ROOT_PARAMETER_OBJECT		1
#define ROOT_PARAMETER_PLAYER		1
#define ROOT_PARAMETER_MATERIAL		2
#define ROOT_PARAMETER_LIGHT		3
#define ROOT_PARAMETER_TEXTURE		4

enum PlayerAni
{
	Idle=0, Move=1, Attack=2, Skill=3, Hitted=4
};

//플레이어 충돌 콜백 함수
//무브함수가 실행될 때만 실행됨
class PlayerCollisionCallback : public PxUserControllerHitReport
{
private:
	Jump * jump;
public:
	void onShapeHit(const PxControllerShapeHit &hit) {
		if (jump->mJump) {
			jump->stopJump();
			//cout << "Hit Shape floor\n";
		}
		

			//cout << "Hit Shape wall\n";
		//cout<<hit.worldPos.x << "  " << hit.worldPos.y << "  " << hit.worldPos.z << endl;
	}
	void 	onControllerHit(const PxControllersHit &hit) {
		//cout << "Hit Other Controller" << endl;
	}
	void 	onObstacleHit(const PxControllerObstacleHit &hit) {
		//cout << "Hit Obstacle" << endl;
	}

	void SetJump(Jump* tmp) {
		jump = tmp;
	}
};

class Player : public ModelObject
{
protected:
	XMFLOAT3				m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3				m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3				m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3				m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	Camera*				m_pCamera;

	PlayerCollisionCallback	m_Callback;
public:
	Player(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Player();

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	PlayerCollisionCallback* getCollisionCallback() { return &m_Callback; }

	void RegenerateMatrix();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	virtual bool Move(DWORD dir, float fDist);
	virtual bool Movement(DWORD input);
	virtual void Animate(float fTime);
	void SetCamera(Camera* tCamera);
};