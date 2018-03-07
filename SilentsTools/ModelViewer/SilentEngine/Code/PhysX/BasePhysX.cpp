#include "stdafx.h"
#include "BasePhysX.h"

BasePhysX::BasePhysX() : gFoundation(nullptr), gPhysics(nullptr),
	gScene(nullptr), gPvd(nullptr)
{
	gTimeStep = 1.0f / 60.0f;  //60프레임
	InitPhysics();
}

BasePhysX::~BasePhysX()
{
}

void BasePhysX::InitPhysics()
{
	//foundation 생성
	gFoundation=PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	
	//PhysXSDK 인스턴스 생성
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());

#ifdef _DEBUG
	if (!gPhysics) {
		cout << "PhysXSDK 생성 실패" << endl;
		exit(1);
	}
#endif

	//scene 생성
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -10.0f, 0.0f); //현실세계 중력가속도 반올림
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1); //scene을 위한 cpuDispatcher 생성
	sceneDesc.filterShader = PxDefaultSimulationFilterShader; //

	gScene = gPhysics->createScene(sceneDesc); //scene 등록

	//physx 매터리얼 생성
	PxMaterial* mat = gPhysics->createMaterial(0.2f, 0.2f, 0.2f);

	//상호작용을 위한 물리객체 생성
	//TEST 용도
	PxTransform planePos = PxTransform(PxVec3(0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidActor* plane = gPhysics->createRigidStatic(planePos);
	plane->createShape(PxPlaneGeometry(), *mat);
	gScene->addActor(*plane); // 엑터에 플레인 등록

	PxTransform tPos(PxVec3(0.0f, 50.0f, 0.0f));
	PxBoxGeometry box(PxVec3(2, 2, 2));
	PxRigidDynamic					*gBox;
	gBox = PxCreateDynamic(*gPhysics, tPos, box, *mat, 1.0f);
	gScene->addActor(*gBox);
}

void BasePhysX::BuildPhysics()
{
	
}


void BasePhysX::stepPhysics(bool interactive)
{
	if (gScene) {
		PX_UNUSED(interactive);
		gScene->simulate(gTimeStep);
		gScene->fetchResults(true); //적용

		/*XMFLOAT3 pos;
		PxRigidActor* tactor;
		gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<PxActor**>(&tactor), 1);
		pos = XMFLOAT3(tactor->getGlobalPose().p.x, tactor->getGlobalPose().p.y, tactor->getGlobalPose().p.z);
		cout << pos.x << "\t" << pos.y << "\t" << pos.z << endl;*/
	}
}

void BasePhysX::cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();

	gFoundation->release();

#ifdef _DEBUG
	cout << "PhysX CleanUp Done" << endl;
#endif
}

void BasePhysX::Addapt(XMFLOAT3 & pos)
{
	//PxU32 tmp = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);

	PxRigidActor* tactor;
	gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<PxActor**>(&tactor), 1);
	pos = XMFLOAT3(tactor->getGlobalPose().p.x, tactor->getGlobalPose().p.y, tactor->getGlobalPose().p.z);
}
