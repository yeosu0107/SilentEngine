#include "stdafx.h"
#include "PhysAddon.h"
#include "..\Shaders\PaticleShader.h"

void PhysSimulation::PlayerToEnemy(PxTriggerPair * trigger)
{
	auto CollisionObject = trigger->otherActor;
	GameObject* enemy = nullptr;
	XMFLOAT3 pos = Vector3::Add(PXtoXM(trigger->triggerActor->getGlobalPose().p), GlobalVal::getInstance()->getPlayer()->GetLook(), 10);
	
	XMFLOAT3 hitPoint[2] = { pos, pos };
	reinterpret_cast<PaticleShader<PaticleObject>*>(GlobalVal::getInstance()->getHitPaticle())
		->SetPos(hitPoint, 2);


	for (UINT i = 0; i < *GlobalVal::getInstance()->getNumEnemy(); ++i) {
		enemy = GlobalVal::getInstance()->getEnemy()[i];
		if (!enemy->isLive())
			continue;
		if (CollisionObject == enemy->getControllerActor()) {
			/*reinterpret_cast<PaticleShader<PaticleObject>*>(GlobalVal::getInstance()->getHitPaticle())
				->SetPos(&Vector3::Add(enemy->GetPosition(), XMFLOAT3(0,20.0f, 0)), 1);*/
			enemy->Hitted();
			return;
		}
	}
}

void PhysSimulation::EnemyToPlayer(PxTriggerPair * trigger)
{
	GameObject* player = GlobalVal::getInstance()->getPlayer();
	if (trigger->otherActor == player->getControllerActor()) {
		player->Hitted(*(float*)(trigger->triggerActor->userData));
	}
	return;
}

void PhysSimulation::onTrigger(PxTriggerPair * pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; ++i) {
		if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
			if(pairs[i].triggerActor ==	//현재 트리거가 플레이어의 트리거일 경우
				GlobalVal::getInstance()->getPlayer()->getTriggerActor())
				PlayerToEnemy(&pairs[i]);
			else {
				EnemyToPlayer(&pairs[i]);
			}
		}
	}
}

Raycast::Raycast(PxGeometry* geom, XMFLOAT3* startPos) :
	m_geom(geom), maxHit(3), m_startPos(startPos), m_closest(50)
{
	hitFlag = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL |
		PxHitFlag::eDISTANCE | PxHitFlag::eUV | PxHitFlag::eMESH_ANY |
		PxHitFlag::eMESH_BOTH_SIDES;
}

Raycast::~Raycast()
{
	delete m_geom;
}

PxAgain Raycast::onHit()
{
	UINT hit = PxGeometryQuery::raycast(XMtoPX(*m_startPos),
		XMtoPX(m_dir), *m_geom, PxTransform(m_pos), m_closest, hitFlag, maxHit, &hitData);

	if (hit != 0 && hitData.distance <= m_closest) {

		return true;
	}
	return false;
}

void Raycast::setPos(PxExtendedVec3 pos)
{
	m_dir = Vector3::Subtract(PXtoXM(pos), *m_startPos, true);
	m_pos = PxVec3(pos.x, pos.y, pos.z);
}

