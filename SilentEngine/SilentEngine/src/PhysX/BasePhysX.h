#pragma once

#include "..\..\PhysX\PxPhysicsAPI.h"
#include "..\Model\LoadModel.h"
using namespace physx;

#define PVD_HOST "127.0.0.1" //���־� ����Ÿ� ���� ������ (���þ�����)

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



enum PhysMesh {
	Mesh_Box=0, Mesh_Capsule=1, Mesh_Tri=2
};

class BasePhysX
{
private:
	PxDefaultAllocator			gAllocator;			//sdk�� �ʿ��� �Ѵ��� �������̽� �⺻ �ν��Ͻ�
	PxDefaultErrorCallback		gErrorCallback;	//���� �ݹ� ����Ʈ

	PxFoundation*				gFoundation;		//�̱��� �Ŀ�̼� SDK Ŭ������ �ν��Ͻ�
	PxPhysics*						gPhysics;			//PhysX SDK �ν��Ͻ�

	PxScene*						gScene;				//PhysX Scene �ν��Ͻ�
	PxReal							gTimeStep;			//�����ӷ���Ʈ

	PxPvd*							gPvd;					//����� �����

	PxCooking*					gCooking;

	PxControllerManager*		gControllerMgr;
	PxCapsuleController*		gPlayer;

public:
	BasePhysX(float frameRate);
	~BasePhysX();

	void InitPhysics();
	void BuildPhysics();

	void stepPhysics(bool interactive);
	void ReleasePhysics(bool interactive);

	void Addapt(XMFLOAT3& pos);

	PxRigidStatic*				GetBoxMesh(PxVec3& t);
	PxTriangleMesh*			GetTriangleMesh(mesh* meshes, UINT count);
	PxCapsuleController*	getCapsuleController(PxExtendedVec3 pos, PxUserControllerHitReport* collisionCallback);
	PxBoxController*			getBoxController(PxExtendedVec3 pos, PxUserControllerHitReport* collisionCallback, float slopeDegree = 0.0f, float step = 0.0f);

	PxPhysics* getPhys() { return gPhysics; }
	PxScene* getScene() { return gScene; }
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