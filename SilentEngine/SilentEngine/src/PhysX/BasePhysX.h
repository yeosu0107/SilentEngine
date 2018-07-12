#pragma once


#include "PhysAddon.h"
#include "..\Model\LoadModel.h"

#define PVD_HOST "127.0.0.1" //비주얼 디버거를 위한 아이피 (로컬아이피)

enum PhysMesh {
	Mesh_Box=0, Mesh_Capsule=1, Mesh_Tri=2
};

class BasePhysX
{
private:
	PxDefaultAllocator			gAllocator;			//sdk에 필요한 한당자 인터페이스 기본 인스턴스
	PxDefaultErrorCallback		gErrorCallback;	//에러 콜백 디폴트

	PxFoundation*				gFoundation;		//싱글톤 파운데이션 SDK 클래스의 인스턴스
	PxPhysics*						gPhysics;			//PhysX SDK 인스턴스

	PxScene*						gScene;				//PhysX Scene 인스턴스
	PxReal							gTimeStep;			//프레임레이트

	PxPvd*							gPvd;					//비쥬얼 디버거

	PxCooking*					gCooking;

	PxControllerManager*		gControllerMgr;
	PxCapsuleController*		gPlayer;

	PhysSimulation				gSimulator;
public:
	BasePhysX(float frameRate);
	~BasePhysX();

	void InitPhysics();
	void BuildPhysics();

	void stepPhysics(bool interactive);

	void ReleasePhysics(bool interactive);

	PxRigidStatic*				GetBoxMesh(PxVec3& t);
	PxTriangleMesh*			GetTriangleMesh(mesh* meshes, UINT count);
	PxCapsuleController*	getCapsuleController(PxExtendedVec3 pos, XMFLOAT2 size, PxUserControllerHitReport* collisionCallback);
	PxBoxController*			getBoxController(PxExtendedVec3 pos, PxUserControllerHitReport* collisionCallback, XMFLOAT3 size, float slopeDegree = 0.0f, float step = 0.0f);

	PxRigidStatic*				getTrigger(PxVec3& t, XMFLOAT3 size);

	PxPhysics* getPhys() { return gPhysics; }
	PxScene* getScene() { return gScene; }

	void registerPlayer(GameObject* object);
};

