#pragma once


#include "PhysAddon.h"
#include "..\Model\LoadModel.h"

#define PVD_HOST "127.0.0.1" //���־� ����Ÿ� ���� ������ (���þ�����)

enum PhysMesh {
	Mesh_Box=0, Mesh_Capsule=1, Mesh_Tri=2
};

class BasePhysX
{
private:
	PxDefaultAllocator			gAllocator;			//sdk�� �ʿ��� �Ѵ��� �������̽� �⺻ �ν��Ͻ�
	PxDefaultErrorCallback		gErrorCallback;	//���� �ݹ� ����Ʈ

	PxFoundation*				gFoundation;		//�̱��� �Ŀ�̼� SDK Ŭ������ �ν��Ͻ�
	PxPhysics*						gPhysics;			//PhysX SDK �ν��Ͻ�

	PxScene*						gScene;				//PhysX Scene �ν��Ͻ�
	PxReal							gTimeStep;			//�����ӷ���Ʈ

	PxPvd*							gPvd;					//����� �����

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

