#pragma once


#include "..\..\PhysX\PxPhysicsAPI.h"
using namespace physx;

#define PVD_HOST "127.0.0.1" //비주얼 디버거를 위한 아이피 (로컬아이피)

class BasePhysX
{
private:
	PxDefaultAllocator			gAllocator; //할당자 인터페이스
	PxDefaultErrorCallback		gErrorCallback; //에러 콜백

	PxFoundation*				gFoundation = nullptr; //
	PxPhysics*						gPhysics = nullptr; //모든 장면에 적용되는 전역 매개변수 설정 및 공유 개체 생성

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