#include "stdafx.h"
#include "PhysAddon.h"
#include "..\Shaders\PaticleShader.h"

unsigned char att_checker = 0;

void PhysSimulation::PlayerToEnemy(PxTriggerPair * trigger)
{
	auto CollisionObject = trigger->otherActor;
	GameObject* enemy = nullptr;
	XMFLOAT3 pos = Vector3::Add(PXtoXM(trigger->triggerActor->getGlobalPose().p), GlobalVal::getInstance()->getPlayer()->GetLook(), 10);
	
	XMFLOAT3 hitPoint[2] = { pos, pos };
	DamageVal* damage = reinterpret_cast<DamageVal*>(trigger->triggerActor->userData);
	
	GlobalVal::getInstance()->setPaticle(damage->paticleType, hitPoint);

	GlobalVal::getInstance()->getSceneCamera()->ShakeInit();

	if (damage->paticleType == 0) {
		att_checker += 1;
		if (att_checker >= 3) {
			att_checker = 0;
			SoundMgr::getInstance()->play(SOUND::HIT02, CHANNEL::FX);
		}
		else {
			SoundMgr::getInstance()->play(SOUND::HIT01, CHANNEL::FX);
		}
	}
	else {
		SOUND fx_index = SOUNDEFFECT(damage->paticleType);
		SoundMgr::getInstance()->play(fx_index, CHANNEL::FX);
	}

	for (UINT i = 0; i < *GlobalVal::getInstance()->getNumEnemy(); ++i) {
		enemy = GlobalVal::getInstance()->getEnemy()[i];
		if (!enemy->isLive())
			continue;
		if (CollisionObject == enemy->getControllerActor()) {
			enemy->Hitted(*damage);
			//enemy->Hitted(10);
			break;
		}
	}
}

void PhysSimulation::EnemyToPlayer(PxTriggerPair * trigger)
{
	if (trigger->otherActor == player->getControllerActor()) {
		player->Hitted(*reinterpret_cast<DamageVal*>(trigger->triggerActor->userData));
		//player->Hitted(10);
	}
}

void PhysSimulation::onTrigger(PxTriggerPair * pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; ++i) {
		if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
			if(pairs[i].triggerActor ==	//현재 트리거가 플레이어의 트리거일 경우
				player->getTriggerActor())
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

