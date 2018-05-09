#include "stdafx.h"
#include "CreepArm.h"

enum ArmAni
{
	armidle = 0, armattack = 1, armdamaged = 2, armdeath = 3
};

CreepArm::CreepArm(LoadModel * model, ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
	: Enemy(model, pd3dDevice, pd3dCommandList)
{
	if (m_State)
		delete m_State;
	m_State = new ArmAI(this);
	m_State->setValue(20, 50, 0, 0, 50, true);
	m_State->setFunc();
	m_status = m_State->getStatus();
	m_size = XMFLOAT2(2.0f, 8.0f);
	m_triggerSize = XMFLOAT3(30, 10, 30);
	SetScale(0.3f);
}

CreepArm::~CreepArm()
{
}

void CreepArm::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	ModelObject::SetAnimations(num, tmp);
}

void CreepArm::Attack()
{
	ChangeAnimation(ArmAni::armattack);
	if (m_loopCheck == LOOP_MID) {
		PxTransform tmpTr(m_Controller->getPosition().x,
			m_Controller->getPosition().y,
			m_Controller->getPosition().z);

		m_attackTrigger->setGlobalPose(tmpTr, true);
	}
}

void CreepArm::Hitted()
{
	ChangeAnimation(ArmAni::armdamaged);
	m_status->m_health -= 10;
	cout << "Enemy Hit!" << "\t";
	cout << "remain HP : " << m_status->m_health << endl;
	m_State->changeState(STATE::hitted);
}

void CreepArm::Death()
{
	ChangeAnimation(ArmAni::armdeath);
}

ArmAI::ArmAI(GameObject * tmp) :
	BaseAI(tmp) { }

void ArmAI::idleState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
	if (recognize(playerPos, m_personalRange)) {
		changeState(STATE::attack);
	}
}

void ArmAI::attackState()
{
	XMFLOAT3 playerPos = GlobalVal::getInstance()->getPlayer()->GetPosition();
	m_owner->Attack();
	if (m_owner->getAnimLoop() == LOOP_END) {
		changeState(STATE::idle);
	}
}

void ArmAI::hittedState()
{
	if (m_owner->getAnimLoop() == LOOP_END)
		changeState(STATE::idle);
}