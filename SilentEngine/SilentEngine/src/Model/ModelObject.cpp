#include "stdafx.h"
#include "ModelObject.h"

static PxF32 gJumpGravity = -50.0f;

Jump::Jump() :
	mV0(0.0f),
	mJumpTime(0.0f),
	mJump(false)
{
}

void Jump::startJump(PxF32 v0)
{
	if (mJump)	return;
	mJumpTime = 0.0f;
	mV0 = v0;
	mJump = true;
}

void Jump::stopJump()
{
	if (!mJump)	return;
	mJump = false;
}

PxF32 Jump::getHeight(PxF32 elapsedTime)
{
	if (!mJump)	return 0.0f;
	mJumpTime += elapsedTime;
	const PxF32 h = gJumpGravity * mJumpTime*mJumpTime + mV0 * mJumpTime;
	return h * elapsedTime;
}

ModelObject::ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : GameObject(model->getNumMesh())
{
	//m_model = model;
	m_model = new LoadModel(*model);
	m_ani = nullptr;
	m_NumofAnim = 0;
	m_AnimIndex = 0;
	m_Animtime = 0.0f;

	//매쉬 적용
	for (UINT i = 0; i < m_nMeshes; ++i) {
		SetMesh(i, m_model->getMeshes()[i]);
	}

	//뼈 정보 초기화
	m_Bones.resize(m_model->GetBones()->size());
	for (auto& p : m_Bones) {
		XMStoreFloat4x4(&p, XMMatrixIdentity());
	}

	m_Controller = nullptr;
	m_Actor = nullptr;
}
ModelObject::~ModelObject()
{
	GameObject::~GameObject();
}

void ModelObject::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	if (num > 0) {
		m_NumofAnim = num;
		m_AnimIndex = 0;
		m_ani = new LoadAnimation*[m_NumofAnim];

		//memcpy(m_ani, tmp, sizeof(LoadAnimation)*num);

		for (UINT i = 0; i < m_NumofAnim; ++i) {
			m_ani[i] = new LoadAnimation(*tmp[i]);
			//memcpy(m_ani[i], tmp[i], sizeof(LoadAnimation));
			//m_ani[i] = tmp[i];
			m_ani[i]->setBones(m_model->GetBones());
		}
	}
}

void ModelObject::Animate(float fTime)
{
	if (m_ani) {
		m_ani[m_AnimIndex]->BoneTransform(m_AnimIndex, m_Bones);
	}
}

void ModelObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	GameObject::Render(pd3dCommandList, pCamera);
}


void ModelObject::SetPhysMesh(BasePhysX* phys, PhysMesh type)
{
	if (type == PhysMesh::Mesh_Tri) {
		PxTriangleMesh* triMesh = phys->GetTriangleMesh(m_model->getMesh(0), m_model->getNumVertices());
		PxVec3 scaleTmp = PxVec3(1.0f, 1.0f, 1.0f);

		PxMeshScale PxScale;
		PxScale.scale = scaleTmp;

		PxTriangleMeshGeometry meshGeo(triMesh, PxScale);
		XMFLOAT3 pos = GetPosition();
		PxTransform location(pos.x, pos.y, pos.z);

		PxMaterial* mat = phys->getPhys()->createMaterial(0.2f, 0.2f, 0.2f);

		m_Actor = PxCreateStatic(*phys->getPhys(), location, meshGeo, *mat);

		m_Actor->setName("tmpmap");
		phys->getScene()->addActor(*m_Actor);
		//phys->getScene()->removeActor
	}
}


void ModelObject::SetPhysController(BasePhysX* control, PxUserControllerHitReport* callback, PxExtendedVec3* pos)
{
	m_Controller = control->getCapsuleController(*pos, callback);
}

void ModelObject::SetActorPos(float xPos, float yPos, float zPos, float rot)
{
	m_Actor->setGlobalPose(PxTransform(xPos, yPos, zPos, PxQuat(XMConvertToRadians(rot), PxVec3(0,1,0))));
	XMFLOAT3 axis(0, 1, 0);
	Rotate(&axis, rot);
	SetPosition(xPos, yPos, zPos);
}

void ModelObject::releasePhys()
{
	if (m_Actor)
		m_Actor->release();
	if (m_Controller)
		m_Controller->release();
}
