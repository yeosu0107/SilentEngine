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

	m_State = new BaseAI(this, 300, true, 0);
	m_State->setFunc();

}

Enemy::~Enemy()
{
	ModelObject::~ModelObject();
}

void Enemy::SetPhysController(BasePhysX * control, PxUserControllerHitReport * callback, PxExtendedVec3 * pos)
{
	ModelObject::SetPhysController(control, &m_Callback, pos);
	m_attackTrigger = control->getTrigger(PxVec3(100,100,100));
}

void Enemy::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
	/*m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Move]->SetAnimSpeed(1.0f);
	m_ani[EnemyAni::Attack]->SetAnimSpeed(0.5f);
	m_ani[EnemyAni::Idle]->SetAnimSpeed(1.0f);*/
	m_ani[EnemyAni::Idle]->EnableLoof();
	m_ani[EnemyAni::Hitted]->SetAnimSpeed(2.0f);
}

void Enemy::Idle()
{
	ChangeAnimation(EnemyAni::Idle);
}

bool Enemy::Move(float fTime)
{
	if (m_Controller) {
		float fDist = -m_moveSpeed * fTime;
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		xmf3Shift = Vector3::Add(xmf3Shift, GetLook(), fDist);

		if (m_Crash) {
			xmf3Shift = Vector3::Add(xmf3Shift, GetRight(), fDist); //맵과 충돌시 장애물 우회
			m_Crash = false;
		}

		m_Controller->move(XMtoPX(xmf3Shift), 0.001f, fTime, m_ControllerFilter);
		ChangeAnimation(EnemyAni::Move);
		return true;
	}
	return false;
}

void Enemy::Attack()
{
	ChangeAnimation(EnemyAni::Attack);
	if (m_loopCheck == LOOP_MID) {

		PxTransform tmpTr(m_Controller->getPosition().x,
			m_Controller->getPosition().y,
			m_Controller->getPosition().z);

		tmpTr = tmpTr.transform(PxTransform(XMtoPX(
			Vector3::ScalarProduct(GetLook(), -30, false)
		)));

		m_attackTrigger->setGlobalPose(tmpTr, true);

	}
}

void Enemy::Skill()
{
	ChangeAnimation(EnemyAni::Skill);
}

void Enemy::Hitted()
{
	ChangeAnimation(EnemyAni::Hitted);
	cout << "Enemy Hit!" << endl;
	m_State->changeState(STATE::hitted);
}

void Enemy::Death()
{
	ChangeAnimation(EnemyAni::Death);
}

void Enemy::Animate(float fTime)
{
	m_attackTrigger->setGlobalPose(PxTransform(100, 100, 100), false);
	m_State->update(fTime);

	ModelObject::Animate(fTime); //애니메이션

	if (m_Controller) {
		//중력작용 처리
		m_Controller->move(PxVec3(0, m_Jump.getHeight(fTime), 0), 0.1f, fTime, m_ControllerFilter);
		//실제 이동
		SetPosition(PXtoXM(m_Controller->getFootPosition()));
		
	}
	if(!m_Jump.mJump)
		m_Jump.startJump(PxF32(0)); //중력 작용
}