#pragma once
#include "..\..\PhysX\PxPhysicsAPI.h"
using namespace physx;

inline PxVec3* fromVertex(vertexDatas* vertex, int size)
{
	PxVec3* mem = new PxVec3[size];
	for (int i = 0; i < size; ++i) {
		mem[i] = PxVec3(vertex[i].m_pos.x, vertex[i].m_pos.y, vertex[i].m_pos.z);
	}
	return mem;
}

inline PxVec3 XMtoPX(const XMFLOAT3& pos) {
	return PxVec3(pos.x, pos.y, pos.z);
}

inline PxExtendedVec3 XMtoPXEx(const XMFLOAT3& pos) {
	return PxExtendedVec3(pos.x, pos.y, pos.z);
}

inline XMFLOAT3 PXtoXM(const PxExtendedVec3& pos) {
	return XMFLOAT3(pos.x, pos.y, pos.z);
}

inline XMFLOAT3 PXtoXM(const PxVec3& pos) {
	return XMFLOAT3(pos.x, pos.y, pos.z);
}

class PhysSimulation : public PxSimulationEventCallback
{
private:
	void PlayerToEnemy(PxTriggerPair* trigger);
	void EnemyToPlayer(PxTriggerPair* trigger);
public:
	void onTrigger(PxTriggerPair* pairs, PxU32 count); //트리거박스 충돌 체크
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}
	void onConstraintBreak(PxConstraintInfo*, PxU32) {  }
	void onWake(PxActor**, PxU32) { }
	void onSleep(PxActor**, PxU32) {  }
	void onContact(const PxContactPairHeader&, const PxContactPair*, PxU32) { }
};

class Raycast
{
private:
	PxGeometry * m_geom;
	PxVec3				m_pos;

	XMFLOAT3*		m_startPos;
	XMFLOAT3			m_dir;
	float					m_closest;

	PxRaycastHit		hitData;
	PxHitFlags			hitFlag;
	int						maxHit;
public:
	Raycast(PxGeometry* geom, XMFLOAT3* startPos);
	~Raycast();

	PxAgain onHit();
	void setPos(PxExtendedVec3 pos);
};