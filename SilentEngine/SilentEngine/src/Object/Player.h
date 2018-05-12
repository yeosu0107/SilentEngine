#pragma once

#include "D3DUtil.h"
#include "..\Model\ModelObject.h"
#include "..\GameLogic\PlayerLogic.h"

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
	Idle=0, Move=1, Attack=2, Skill=3, Hitted=4, die=5
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
		}
	}
	void 	onControllerHit(const PxControllersHit &hit) {	}
	void 	onObstacleHit(const PxControllerObstacleHit &hit) {	}

	void SetJump(Jump* tmp) {
		jump = tmp;
	}
};

class CameraCollisionCallback : public PxUserControllerHitReport
{
private:
	PxControllerFilters		m_ControllerFilter;
	float							m_crashMove = 3.0f;

public:
	void onShapeHit(const PxControllerShapeHit &hit) {
		PxVec3 tmp = hit.dir * m_crashMove;
	}
	void 	onControllerHit(const PxControllersHit &hit) {

	}
	void 	onObstacleHit(const PxControllerObstacleHit &hit) {
		
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
	PxBoxController*		m_cameraController;
	PxRigidActor*			m_weaponTrigger;

	Jump							m_Jump;

	PlayerCollisionCallback	m_Callback;
	CameraCollisionCallback	m_CameraCallback;

	PlayerLogic*					m_playerLogic;

	float								hitBackstep = 0.0f;

	bool								m_avoid = false;
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


	virtual bool Move(DWORD dir, float fTime);
	virtual bool Movement(DWORD input);
	virtual void Idle();
	virtual void Attack();
	virtual void Hitted(float& hitback);
	virtual void Skill();

	virtual void SetPosition(float x, float y, float z);
	virtual void Animate(float fTime);

	void SetCamera(Camera* tCamera, BasePhysX* phys);
	void CalibrateLook(XMFLOAT3& look);
	Status* GetStatus() { return m_status; }
	void* getTriggerActor() { return m_weaponTrigger; }
};