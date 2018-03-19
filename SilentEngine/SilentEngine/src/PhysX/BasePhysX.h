#pragma once

#include "..\..\PhysX\PxPhysicsAPI.h"
#include "..\Model\LoadModel.h"
using namespace physx;

#define PVD_HOST "127.0.0.1" //비주얼 디버거를 위한 아이피 (로컬아이피)

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