#pragma once

#include "..\..\PhysX\PxPhysicsAPI.h"
using namespace physx;

#define PVD_HOST "127.0.0.1" //���־� ����Ÿ� ���� ������ (���þ�����)

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

	PxPhysics* getPhys() { return gPhysics; }
	PxScene* getScene() { return gScene; }
};