#pragma once


#include "..\..\PhysX\PxPhysicsAPI.h"
using namespace physx;

#define PVD_HOST "127.0.0.1" //���־� ����Ÿ� ���� ������ (���þ�����)

class BasePhysX
{
private:
	PxDefaultAllocator			gAllocator; //�Ҵ��� �������̽�
	PxDefaultErrorCallback		gErrorCallback; //���� �ݹ�

	PxFoundation*				gFoundation = nullptr; //
	PxPhysics*						gPhysics = nullptr; //��� ��鿡 ����Ǵ� ���� �Ű����� ���� �� ���� ��ü ����

	PxDefaultCpuDispatcher*	gDispatcher = nullptr;
	PxScene*						gScene = nullptr;

	PxMaterial*						gMaterial = nullptr;

	PxPvd*							gPvd = nullptr;

	PxReal							stackZ = 10.0f;

public:
	BasePhysX();
	~BasePhysX();

	void InitPhysics(bool interactive);

	PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry,
		const PxVec3& velocity);
};