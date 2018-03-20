#include "stdafx.h"
#include "ModelObject.h"

ModelObject::ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : GameObject(model->getNumMesh())
{
	m_model = model;
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
		for (UINT i = 0; i < m_NumofAnim; ++i) {
			m_ani[i] = tmp[i];
			m_ani[i]->setBones(m_model->GetBones());
		}
	}
}

void ModelObject::Animate(float fTime)
{
	if (m_ani) {
		m_AnimIndex = 0;
		m_ani[m_AnimIndex]->BoneTransform(m_AnimIndex, m_Bones);
		//m_Animtime += 0.03f;
		
	}
	/*if (m_Controller) {
		m_Controller->move(PxVec3(0, 1, 0)*-0.1f, 0.001f, 1, m_ControllerFilter);
		XMFLOAT3 tt = XMFLOAT3(m_Controller->getPosition().x, m_Controller->getPosition().y, m_Controller->getPosition().z);
		SetPosition(tt);
	}*/
}

void ModelObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	GameObject::Render(pd3dCommandList, pCamera);
}


void ModelObject::SetPhysMesh(BasePhysX* phys, PhysMesh type)
{
	if (type == PhysMesh::Mesh_Tri) {
		PxTriangleMesh* triMesh = phys->GetTriangleMesh(m_model->getMesh(0), m_model->getNumVertices());
		PxVec3 scaleTmp = PxVec3(0.1f, 0.1f, 0.1f);

		PxMeshScale PxScale;
		PxScale.scale = scaleTmp;

		PxTriangleMeshGeometry meshGeo(triMesh, PxScale);
		PxTransform location(0, 0, 0);

		PxMaterial* mat = phys->getPhys()->createMaterial(0.2f, 0.2f, 0.2f);

		PxRigidActor* actor = PxCreateStatic(*phys->getPhys(), location, meshGeo, *mat);

		actor->setName("tmpmap");

		phys->getScene()->addActor(*actor);
	}
}


void ModelObject::SetPhysController(PxCapsuleController * control)
{
	m_Controller = control;
}
