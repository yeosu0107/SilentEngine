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

	PxTriangleMesh* GetTriangleMesh(mesh* meshes, UINT count);

	PxPhysics* getPhys() { return gPhysics; }
	PxScene* getScene() { return gScene; }
};