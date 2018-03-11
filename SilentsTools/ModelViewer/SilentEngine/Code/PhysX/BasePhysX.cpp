#include "stdafx.h"
#include "BasePhysX.h"

BasePhysX::BasePhysX() : gFoundation(nullptr), gPhysics(nullptr),
	gScene(nullptr), gPvd(nullptr), gControllerMgr(nullptr)
{
	gTimeStep = 1.0f / 60.0f;  //60������
	InitPhysics();
}

BasePhysX::~BasePhysX()
{
}

void BasePhysX::InitPhysics()
{
	//foundation ����
	gFoundation=PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	
	//PhysXSDK �ν��Ͻ� ����
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());

#ifdef _DEBUG
	if (!gPhysics) {
		cout << "PhysXSDK ���� ����" << endl;
		exit(1);
	}
#endif

	//scene ����
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -10.0f, 0.0f); //���Ǽ��� �߷°��ӵ� �ݿø�
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1); //scene�� ���� cpuDispatcher ����
	sceneDesc.filterShader = PxDefaultSimulationFilterShader; //

	gScene = gPhysics->createScene(sceneDesc); //scene ���

	//��Ʈ�ѷ� ����
	gControllerMgr = PxCreateControllerManager(*gScene);
#ifdef _DEBUG
	if (!gControllerMgr)
		cout << "��Ʈ�ѷ� ���� ����" << endl;
#endif

	//ĸ�� ��Ʈ�ѷ� �κ�. ���� �� �÷��̾� ������Ʈ�� �̽��ؾ� �� ��
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.height = 1; //Height of capsule
	capsuleDesc.radius = 2; //Radius of casule
	capsuleDesc.position = PxExtendedVec3(0, 0, 0); //Initial position of capsule
	capsuleDesc.material = gPhysics->createMaterial(0.2f, 0.2f, 0.2f); //Material for capsule shape
	capsuleDesc.density = 1.0f; //Desity of capsule shape
	capsuleDesc.contactOffset = 0.05f;
	capsuleDesc.slopeLimit = 0.2f;
	capsuleDesc.stepOffset = 0.75f;

	gPlayer = static_cast<PxCapsuleController*>(gControllerMgr->createController(capsuleDesc));

	//physx ���͸��� ����
	PxMaterial* mat = gPhysics->createMaterial(0.2f, 0.2f, 0.2f);

	//��ȣ�ۿ��� ���� ������ü ����
	//TEST �뵵
	PxTransform planePos = PxTransform(PxVec3(0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidActor* plane = gPhysics->createRigidStatic(planePos);
	plane->createShape(PxPlaneGeometry(), *mat);
	gScene->addActor(*plane); // ���Ϳ� �÷��� ���

}

void BasePhysX::BuildPhysics()
{
	
}


void BasePhysX::stepPhysics(bool interactive)
{
	if (gScene) {
		PX_UNUSED(interactive);
		gScene->simulate(gTimeStep);
		gScene->fetchResults(true); //����

		//cout << gPlayer->getPosition().x << "\t" << gPlayer->getPosition().y << "\t" << gPlayer->getPosition().z << endl;
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
