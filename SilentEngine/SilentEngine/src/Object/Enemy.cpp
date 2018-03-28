#include "stdafx.h"
#include "Enemy.h"

Bullet::Bullet()
{
	m_live = false;
}

Bullet::~Bullet()
{
}

void Bullet::Animate()
{
	MoveForward(2.0f);
}

void Bullet::Shoot(XMFLOAT3 pos, XMFLOAT3 target)
{
	m_live = true;
	SetPosition(pos);
	SetLookAt(target);
}

Enemy::Enemy(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: ModelObject(model, pd3dDevice, pd3dCommandList), 
	m_Speed(1.0f), m_Crash(false)
{
	m_Callback.SetJump(&m_Jump);
	m_Callback.SetCrash(&m_Crash);
	//Rotate(0, 180, 0);
}

Enemy::~Enemy()
{
	ModelObject::~ModelObject();
}

void Enemy::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Move]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Attack]->SetAnimSpeed(0.5f);
	m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);
}

bool Enemy::Move(float fDist, float fTime)
{
	if (m_Controller) {
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		xmf3Shift = Vector3::Add(xmf3Shift, GetLook(), fDist);
		m_Controller->move(XMtoPX(xmf3Shift), 1.0f, fTime, m_ControllerFilter);
		m_AnimIndex = EnemyAni::Move;
		return true;
	}
	return false;
}

void Enemy::Animate(float fTime)
{
	m_AnimIndex = EnemyAni::Idle;
	Move(-2.0f, fTime);
	
	ModelObject::Animate(fTime); //애니메이션
	if (m_Crash) {
		Rotate(0, 90, 0);
		m_Crash = false;
	}
	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(1.0f / 60.0f), 0), 0.1f, 1.0f / 60.0f, m_ControllerFilter);
		//실제 이동
		SetPosition(PXtoXM(m_Controller->getFootPosition()));
	}
	if(!m_Jump.mJump)
		m_Jump.startJump(PxF32(0)); //중력 작용
}


