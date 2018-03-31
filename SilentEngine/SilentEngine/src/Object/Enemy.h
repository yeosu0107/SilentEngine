#pragma once

#include "D3DUtil.h"
#include "..\Model\ModelObject.h"

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
		*crash = true;
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

class Bullet : public GameObject
{
private:
	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dEffectCbvGPUDescriptorHandle;
	UINT																m_nInstanceCount;

	XMFLOAT3														m_moveDir;
public:
	Bullet();
	~Bullet();

	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetEffectCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dEffectCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetEffectCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dEffectCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }

	virtual void SetInstanceCount(UINT count) { m_nInstanceCount = count; }

	virtual void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	virtual void Animate(float fTimeElapsed);
	
	void Shoot(XMFLOAT3 pos, XMFLOAT3 target);
};

class Enemy : public ModelObject
{
private:
	EnemyCollisionCallback	m_Callback;
	float								m_Speed;
	bool								m_Crash;

	Jump							m_Jump;
public:
	Enemy(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Enemy();
	
	EnemyCollisionCallback* getCollisionCallback() { return &m_Callback; }

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);

	bool Move(float fDist, float fTime);
	virtual void Animate(float fTime);
};