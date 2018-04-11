#include "stdafx.h"
#include "Enemy.h"

string* name = new string("bullet");

Bullet::Bullet()
{
	m_live = false;
	m_crash = false;
	m_moveSpeed = 200.0f;
	m_timer = 0;
	m_crashPos = XMFLOAT3(0, 0, 0);
	m_moveDir = XMFLOAT3(0, 0, 0);
	m_Callback.SetCrash(&m_crash, &m_crashPos);
}

Bullet::~Bullet()
{
}

void Bullet::Animate(float fTimeElapsed)
{
	if (!m_live) return;

	if (m_crash) {
		releasePhys();
		m_live = false;
		m_crash = false;
		return;
	}

	EffectInstanceObject::Animate(fTimeElapsed);

	m_Controller->move(XMtoPX(m_moveDir) * m_moveSpeed * fTimeElapsed, 0.1f, 1, m_ControllerFilter);
	SetPosition(PXtoXM(m_Controller->getPosition()));
	m_timer += 1;
	if (m_timer >= MAX_BULLET_TIME) {
		m_live = false;
		m_timer = 0;
		releasePhys();
	}
}

void Bullet::Shoot(BasePhysX* phys, XMFLOAT3 pos, XMFLOAT3 target)
{
	m_live = true;
	m_crash = false;
	m_timer = 0;
	m_fNowXCount = 0.0f;
	m_fNowYCount = 0.0f;
	SetPosition(pos);
	m_moveDir = Vector3::Subtract(target, pos, true);
	m_Controller = phys->getBoxController(XMtoPXEx(pos), &m_Callback, name);
	//if (m_Controller)
	//	cout << *(string*)(m_Controller->getUserData()) << endl;
}

void Bullet::releasePhys()
{
	if (m_Controller)
		m_Controller->release();
}

Enemy::Enemy(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: ModelObject(model, pd3dDevice, pd3dCommandList), 
	m_Crash(false)
{
	m_Callback.SetJump(&m_Jump);
	m_Callback.SetCrash(&m_Crash);
	m_moveSpeed = 100.0f;
}

Enemy::~Enemy()
{
	ModelObject::~ModelObject();
}

void Enemy::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	/*m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Move]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Attack]->SetAnimSpeed(0.5f);
	m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);*/
}

bool Enemy::Move(float fTime)
{
	if (m_Controller) {
		float fDist = -m_moveSpeed * fTime;
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
	Move(fTime);
	
	ModelObject::Animate(fTime); //애니메이션
	if (m_Crash) {
		Rotate(0, 90, 0);
		m_Crash = false;
	}
	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(fTime), 0), 0.1f, fTime, m_ControllerFilter);
		//실제 이동
		SetPosition(PXtoXM(m_Controller->getFootPosition()));
	}
	if(!m_Jump.mJump)
		m_Jump.startJump(PxF32(0)); //중력 작용
}


