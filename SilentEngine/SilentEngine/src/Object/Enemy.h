#pragma once

#include "D3DUtil.h"
#include "..\Model\ModelObject.h"
#include "..\Object\InstanceObject.h"
#include "..\GameLogic\StateMachine.h"


const UINT MAX_BULLET_TIME = 600;

enum EnemyAni
{
	Idle = 0, Move = 1, Attack = 2, Skill = 3, Hitted = 4
};

class EnemyCollisionCallback : public PxUserControllerHitReport
{
private:
	Jump * jump;
	bool* crash;
public:
	void onShapeHit(const PxControllerShapeHit &hit) {
		if (jump->mJump) {
			jump->stopJump();
		}
		if (hit.dir.x != 0 || hit.dir.z != 0) {
			*crash = true;
		}
	}
	void 	onControllerHit(const PxControllersHit &hit) {
		//*crash = true;
		//cout << "Crash!" << endl;
	}
	void 	onObstacleHit(const PxControllerObstacleHit &hit) {
	}
	void SetJump(Jump* tmp) {
		jump = tmp;
	}
	void SetCrash(bool* tmp) {
		crash = tmp;
	}
};

class Bullet;

class ProjectileCollisionCallback : public PxUserControllerHitReport
{
private:
	bool* crash;
	XMFLOAT3* crashPos;
public:
	void onShapeHit(const PxControllerShapeHit &hit) {
		//cout << "shape" << endl;
		*crashPos = PXtoXM(hit.worldPos);
		*crash = true;

	}
	void 	onControllerHit(const PxControllersHit &hit) {
		*crashPos = PXtoXM(hit.worldPos);
		*crash = true;
		
		//hit는 자기자신
		//hit.other는 나와 부딪친 객체
		//cout<<*(string*)(hit.other->getUserData());
	}
	void 	onObstacleHit(const PxControllerObstacleHit &hit) {
	}

	void SetCrash(bool* tmp, XMFLOAT3* pos) {
		crash = tmp;
		crashPos = pos;
	}
};

class Bullet : public EffectInstanceObject
{
private:
	ProjectileCollisionCallback								m_Callback;

	XMFLOAT3														m_moveDir;
	XMFLOAT3														m_crashPos;
	bool																m_crash;
	
	UINT																m_timer;

	PxBoxController*												m_Controller;
	PxControllerFilters											m_ControllerFilter;
public:
	Bullet();
	~Bullet();


	ProjectileCollisionCallback* getCollisionCallback() { return &m_Callback; }
	XMFLOAT3 getCrashPos() const { return m_crashPos; }

	virtual void Animate(float fTimeElapsed);
	XMFLOAT3 returnCollisionPos() const { return m_crashPos; }
	
	bool isCrash() const { return m_crash; }

	void Shoot(BasePhysX* phys, XMFLOAT3 pos, XMFLOAT3 target);

	void releasePhys();
};

class Enemy : public ModelObject
{
private:
	EnemyCollisionCallback	m_Callback;
	bool								m_Crash;

	Jump								m_Jump;
	BaseAI*							m_State = nullptr;
public:
	Enemy(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Enemy();
	
	EnemyCollisionCallback* getCollisionCallback() { return &m_Callback; }

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	virtual bool Move(float fTime);
	virtual void Attack();
	virtual void Skill();

	virtual void Animate(float fTime);
};