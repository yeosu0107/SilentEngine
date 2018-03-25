#include "stdafx.h"
#include "BasePhysX.h"

PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return PxFilterFlag::eDEFAULT;
}

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

	//비주얼디버거 세팅
	PxPvdTransport* tmp = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	//PxPvdTransport* tmp = PxDefaultPvdFileTransportCreate("C:\\Users\\yeosu\\OneDrive\\Documents\\Project\\ModelViewer\\SilentEngine\\sample.pxd2");

	PxPvdInstrumentationFlags flag = PxPvdInstrumentationFlag::eALL;

	gPvd = PxCreatePvd(*gFoundation);
	gPvd->connect(*tmp, flag);

	//PhysXSDK 인스턴스 생성
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

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
	//sceneDesc.filterShader = contactReportFilterShader;
	//sceneDesc.simulationEventCallback = &gCallback;	//충돌 콜백

	gScene = gPhysics->createScene(sceneDesc); //scene 등록

	//Cooking 생성
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, * gFoundation, PxCookingParams(PxTolerancesScale()));

	 //컨트롤러 생성
	gControllerMgr = PxCreateControllerManager(*gScene);
#ifdef _DEBUG
	if (!gControllerMgr)
		cout << "컨트롤러 생성 실패" << endl;
#endif

	//physx 매터리얼 생성
	PxMaterial* mat = gPhysics->createMaterial(0.2f, 0.2f, 0.2f);

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
	
	meshDesc.triangles.count = meshes->m_indices.size()/3;
	meshDesc.triangles.stride = sizeof(int) * 3;
	meshDesc.triangles.data = meshes->m_indices.data();

	meshDesc.flags = PxMeshFlags(0);
	
	PxCookingParams params = gCooking->getParams();
	params.midphaseDesc = PxMeshMidPhase::eBVH33;
	params.suppressTriangleMeshRemapTable = true;
	params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
	params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
	params.midphaseDesc.mBVH33Desc.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
	params.midphaseDesc.mBVH33Desc.meshSizePerformanceTradeOff = 0.0f;
	gCooking->setParams(params);

	PxTriangleMesh* triMesh = nullptr;
	//triMesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());
	
	//PxU32 meshSize = 0;
	
	PxDefaultMemoryOutputStream outBuffer;
	gCooking->cookTriangleMesh(meshDesc, outBuffer);

	PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
	triMesh = gPhysics->createTriangleMesh(stream);
	//meshSize = outBuffer.getSize();

	return triMesh;
}

PxCapsuleController* BasePhysX::getCapsuleController(PxUserControllerHitReport* collisionCallback)
{
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.height = 0.3f; //Height of capsule
	capsuleDesc.radius = 5.0f; //Radius of casule
	capsuleDesc.position = PxExtendedVec3(0, 0, 0); //Initial position of capsule
	capsuleDesc.material = gPhysics->createMaterial(1.0f,1.0f, 1.0f); //Material for capsule shape
	capsuleDesc.density = 1.0f; //Desity of capsule shape
	capsuleDesc.contactOffset = 1.01f; //외부 물체와 상호작용하는 크기 (지정한 충돌캡슐보다 조금 더 크게 형성위해)
	capsuleDesc.slopeLimit = cosf(XMConvertToRadians(15.0f)); //경사 허용도(degree) 0에 가까울수록 경사를 못올라감
	capsuleDesc.stepOffset = 1.0f;	//자연스러운 이동 (약간의 고저에 부딫혔을 때 이동가능 여부)
													//stepoffset보다 큰 높이에 부딛치면 멈춤
	//capsuleDesc.maxJumpHeight = 2.0f; //최대 점프 높이
	capsuleDesc.climbingMode = PxCapsuleClimbingMode::eEASY;
	//capsuleDesc.invisibleWallHeight = 0.0f;

	//충돌 콜백 함수
	capsuleDesc.reportCallback = collisionCallback;

	PxCapsuleController* controller = static_cast<PxCapsuleController*>(gControllerMgr->createController(capsuleDesc));

	return controller;
}

PxBoxController* BasePhysX::getBoxController(PxUserControllerHitReport * collisionCallback)
{
	PxBoxControllerDesc boxDesc;
	boxDesc.halfForwardExtent = 1.5f;
	boxDesc.halfHeight = 1.5f;
	boxDesc.halfSideExtent = 1.5f;

	boxDesc.position = PxExtendedVec3(0, 0, 0);

	boxDesc.density = 1.0f;
	boxDesc.material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);
	boxDesc.contactOffset = 5.0f;
	boxDesc.slopeLimit = 0.0f;

	boxDesc.reportCallback = collisionCallback;

	PxBoxController* controller = static_cast<PxBoxController*>(gControllerMgr->createController(boxDesc));
	
	return controller;
}
