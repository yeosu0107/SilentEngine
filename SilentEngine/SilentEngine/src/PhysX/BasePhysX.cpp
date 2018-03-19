#include "stdafx.h"
#include "BasePhysX.h"

BasePhysX::BasePhysX(float frameRate) : gFoundation(nullptr), gPhysics(nullptr),
gScene(nullptr), gPvd(nullptr), gControllerMgr(nullptr)
{
	gTimeStep = 1.0f / frameRate;  
	InitPhysics();
}

BasePhysX::~BasePhysX()
{
}

void BasePhysX::InitPhysics()
{
	//foundation 생성
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

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

	//Cooking 생성
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, * gFoundation, PxCookingParams(PxTolerancesScale()));

	 //컨트롤러 생성
	gControllerMgr = PxCreateControllerManager(*gScene);
#ifdef _DEBUG
	if (!gControllerMgr)
		cout << "컨트롤러 생성 실패" << endl;
#endif

	//캡슐 컨트롤러 부분. 추후 각 플레이어 오브젝트로 이식해야 할 듯
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

	//physx 매터리얼 생성
	PxMaterial* mat = gPhysics->createMaterial(0.2f, 0.2f, 0.2f);

	//상호작용을 위한 물리객체 생성
	//TEST 용도
	PxTransform planePos = PxTransform(PxVec3(0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidActor* plane = gPhysics->createRigidStatic(planePos);
	plane->createShape(PxPlaneGeometry(), *mat);
	gScene->addActor(*plane); // 엑터에 플레인 등록


	//비주얼디버거 세팅
	PxPvdTransport* tmp = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	//PxPvdTransport* tmp = PxDefaultPvdFileTransportCreate("C:\\Users\\yeosu\\OneDrive\\Documents\\Project\\ModelViewer\\SilentEngine\\sample.pxd2");

	PxPvdInstrumentationFlags flag = PxPvdInstrumentationFlag::eALL;

	gPvd = PxCreatePvd(*gFoundation);
	gPvd->connect(*tmp, flag);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
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

		//cout << gPlayer->getPosition().x << "\t" << gPlayer->getPosition().y << "\t" << gPlayer->getPosition().z << endl;
		XMFLOAT3 pos;
		PxRigidActor* tactor;
		gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<PxActor**>(&tactor), 1);
		pos = XMFLOAT3(tactor->getGlobalPose().p.x, tactor->getGlobalPose().p.y, tactor->getGlobalPose().p.z);
		//cout << pos.x << "\t" << pos.y << "\t" << pos.z << endl;
	}
}

void BasePhysX::ReleasePhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();

	gFoundation->release();
	gCooking->release();

#ifdef _DEBUG
	cout << "PhysX CleanUp Done" << endl;
#endif
}

void BasePhysX::Addapt(XMFLOAT3 & pos)
{
	PxRigidActor* tactor;
	gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<PxActor**>(&tactor), 1);
	pos = XMFLOAT3(tactor->getGlobalPose().p.x, tactor->getGlobalPose().p.y, tactor->getGlobalPose().p.z);
}

PxTriangleMesh * BasePhysX::GetTriangleMesh(mesh* meshes, UINT count)
{
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = meshes->m_vertices.size();
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = fromVertex(meshes->m_vertices.data(), meshes->m_vertices.size());
	
	meshDesc.triangles.count = meshes->m_indices.size();
	meshDesc.triangles.stride = sizeof(int) * 3;
	meshDesc.triangles.data = meshes->m_indices.data();

	meshDesc.flags = PxMeshFlags(0);

	PxDefaultMemoryOutputStream stream;
	

	return nullptr;
}
