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
	//시뮬레이터 이벤트콜백 생성
	//gSimulator = new PhysSimulation();
	//scene 생성
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -10.0f, 0.0f); //현실세계 중력가속도 반올림
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1); //scene을 위한 cpuDispatcher 생성
	sceneDesc.filterShader = PxDefaultSimulationFilterShader; //
	//sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gSimulator;	//충돌 콜백

	gScene = gPhysics->createScene(sceneDesc); //scene 등록

	//Cooking 생성
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, * gFoundation, PxCookingParams(PxTolerancesScale()));

	 //컨트롤러 생성
	gControllerMgr = PxCreateControllerManager(*gScene);
#ifdef _DEBUG
	if (!gControllerMgr)
		cout << "컨트롤러 생성 실패" << endl;
#endif

	
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

PxRigidStatic * BasePhysX::GetBoxMesh(PxVec3& t)
{
	//physx 매터리얼 생성
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(5.0f, 5.0f, 5.0f), *gPhysics->createMaterial(0.2f, 0.2f, 0.2f));
	PxRigidStatic* body =  gPhysics->createRigidStatic(PxTransform(t.x, t.y, t.z));
	body->attachShape(*shape);
	body->setName("ttt");
	gScene->addActor(*body);
	return body;
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

PxCapsuleController* BasePhysX::getCapsuleController(PxExtendedVec3 pos, XMFLOAT2 size, PxUserControllerHitReport* collisionCallback)
{
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.height = size.x; //Height of capsule
	capsuleDesc.radius = size.y; //Radius of casule
	capsuleDesc.position = pos; //Initial position of capsule
	capsuleDesc.material = gPhysics->createMaterial(1.0f,1.0f, 1.0f); //Material for capsule shape
	capsuleDesc.density = 1.0f; //Desity of capsule shape
	capsuleDesc.contactOffset = 1.0f; //외부 물체와 상호작용하는 크기 (지정한 충돌캡슐보다 조금 더 크게 형성위해)
	capsuleDesc.slopeLimit = cosf(XMConvertToRadians(1.0f)); //경사 허용도(degree) 0에 가까울수록 경사를 못올라감
	capsuleDesc.stepOffset = 0.0f;	//자연스러운 이동 (약간의 고저에 부딫혔을 때 이동가능 여부)
													//stepoffset보다 큰 높이에 부딛치면 멈춤
	//capsuleDesc.maxJumpHeight = 2.0f; //최대 점프 높이
	capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	//capsuleDesc.invisibleWallHeight = 0.0f;

	//충돌 콜백 함수
	capsuleDesc.reportCallback = collisionCallback;

	PxCapsuleController* controller = static_cast<PxCapsuleController*>(gControllerMgr->createController(capsuleDesc));

	return controller;
}

PxBoxController* BasePhysX::getBoxController(PxExtendedVec3 pos, PxUserControllerHitReport * collisionCallback, XMFLOAT3 size, float slopeDegree, float step)
{
	PxBoxControllerDesc boxDesc;
	boxDesc.halfForwardExtent = size.x;
	boxDesc.halfHeight = size.y;
	boxDesc.halfSideExtent = size.z;

	boxDesc.position = pos;

	boxDesc.density = 1.0f;
	boxDesc.material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);
	boxDesc.contactOffset = 5.0f;
	boxDesc.slopeLimit = slopeDegree;
	boxDesc.stepOffset = step;

	boxDesc.reportCallback = collisionCallback;

	PxBoxController* controller = static_cast<PxBoxController*>(gControllerMgr->createController(boxDesc));

	return controller;
}

PxRigidStatic * BasePhysX::getTrigger(PxVec3 & t, XMFLOAT3 size)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(size.x, size.y, size.z), *gPhysics->createMaterial(0.2f, 0.2f, 0.2f));
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);	//시물레이션 off
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);			//트리거링 on

	PxRigidStatic * staticActor = gPhysics->createRigidStatic(PxTransform(t));
	staticActor->attachShape(*shape);
	float* num = new float(0.0f);
	staticActor->userData = (void*)num;
	/*int* tmp = (int*)staticActor->userData;
	*tmp = 1;*/
	gScene->addActor(*staticActor);
	return staticActor;
}

PxShape * BasePhysX::getShape(XMFLOAT3 size)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(size.x, size.y, size.z), *gPhysics->createMaterial(0.2f, 0.2f, 0.2f));
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);	//시물레이션 off
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);			//트리거링 on

	return shape;
}

void BasePhysX::registerPlayer(GameObject * object)
{
	gSimulator.setPlayer(object);
}

