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

	//foundation ����
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	//���־����� ����
	PxPvdTransport* tmp = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	//PxPvdTransport* tmp = PxDefaultPvdFileTransportCreate("C:\\Users\\yeosu\\OneDrive\\Documents\\Project\\ModelViewer\\SilentEngine\\sample.pxd2");

	PxPvdInstrumentationFlags flag = PxPvdInstrumentationFlag::eALL;

	gPvd = PxCreatePvd(*gFoundation);
	gPvd->connect(*tmp, flag);

	//PhysXSDK �ν��Ͻ� ����
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

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
	//sceneDesc.filterShader = contactReportFilterShader;
	//sceneDesc.simulationEventCallback = &gCallback;	//�浹 �ݹ�

	gScene = gPhysics->createScene(sceneDesc); //scene ���

	//Cooking ����
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, * gFoundation, PxCookingParams(PxTolerancesScale()));

	 //��Ʈ�ѷ� ����
	gControllerMgr = PxCreateControllerManager(*gScene);
#ifdef _DEBUG
	if (!gControllerMgr)
		cout << "��Ʈ�ѷ� ���� ����" << endl;
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
		gScene->fetchResults(true); //����
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

PxRigidStatic * BasePhysX::GetBoxMesh(PxVec3& t)
{
	//physx ���͸��� ����
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

PxCapsuleController* BasePhysX::getCapsuleController(PxExtendedVec3 pos, PxUserControllerHitReport* collisionCallback, string* name)
{
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.height = 0.3f; //Height of capsule
	capsuleDesc.radius = 15.0f; //Radius of casule
	capsuleDesc.position = pos; //Initial position of capsule
	capsuleDesc.material = gPhysics->createMaterial(1.0f,1.0f, 1.0f); //Material for capsule shape
	capsuleDesc.density = 1.0f; //Desity of capsule shape
	capsuleDesc.contactOffset = 1.01f; //�ܺ� ��ü�� ��ȣ�ۿ��ϴ� ũ�� (������ �浹ĸ������ ���� �� ũ�� ��������)
	capsuleDesc.slopeLimit = cosf(XMConvertToRadians(0.0f)); //��� ��뵵(degree) 0�� �������� ��縦 ���ö�
	capsuleDesc.stepOffset = 3.0f;	//�ڿ������� �̵� (�ణ�� ������ �΋H���� �� �̵����� ����)
													//stepoffset���� ū ���̿� �ε�ġ�� ����
	//capsuleDesc.maxJumpHeight = 2.0f; //�ִ� ���� ����
	capsuleDesc.climbingMode = PxCapsuleClimbingMode::eEASY;
	//capsuleDesc.invisibleWallHeight = 0.0f;

	//�浹 �ݹ� �Լ�
	capsuleDesc.reportCallback = collisionCallback;
	
	PxCapsuleController* controller = static_cast<PxCapsuleController*>(gControllerMgr->createController(capsuleDesc));
;
	controller->setUserData(name);
	return controller;
}

PxBoxController* BasePhysX::getBoxController(PxExtendedVec3 pos, PxUserControllerHitReport * collisionCallback, string* name, float slopeDegree, float step)
{
	PxBoxControllerDesc boxDesc;
	boxDesc.halfForwardExtent = 5.5f;
	boxDesc.halfHeight = 5.5f;
	boxDesc.halfSideExtent = 5.5f;

	boxDesc.position = pos;

	boxDesc.density = 1.0f;
	boxDesc.material = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);
	boxDesc.contactOffset = 5.0f;
	boxDesc.slopeLimit = slopeDegree;
	boxDesc.stepOffset = step;

	boxDesc.reportCallback = collisionCallback;

	PxBoxController* controller = static_cast<PxBoxController*>(gControllerMgr->createController(boxDesc));

	controller->setUserData(name);
	return controller;
}

Raycast::Raycast(PxGeometry* geom, XMFLOAT3* startPos) :
	m_geom(geom), maxHit(3), m_startPos(startPos), m_closest(50)
{
	hitFlag = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL |
		PxHitFlag::eDISTANCE | PxHitFlag::eUV | PxHitFlag::eMESH_ANY |
		PxHitFlag::eMESH_BOTH_SIDES;
}

Raycast::~Raycast()
{
	delete m_geom;
}

PxAgain Raycast::onHit()
{
	UINT hit = PxGeometryQuery::raycast(XMtoPX(*m_startPos),
		XMtoPX(m_dir), *m_geom, PxTransform(m_pos), m_closest, hitFlag, maxHit, &hitData);

	if (hit != 0 && hitData.distance <= m_closest) {

		return true;
	}
	return false;
}

void Raycast::setPos(PxExtendedVec3 pos)
{
	m_dir = Vector3::Subtract(PXtoXM(pos), *m_startPos, true);
	m_pos = PxVec3(pos.x, pos.y, pos.z);
}