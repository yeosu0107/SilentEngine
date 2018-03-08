#include "stdafx.h"
#include "ModelObject.h"

ModelObject::ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CGameObject(model->getNumMesh())
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

	physBox = nullptr;
	tmp = nullptr;
}
ModelObject::~ModelObject() 
{
	CGameObject::~CGameObject();
}

void ModelObject::SetAnimations(UINT num, LoadAnimation ** tmp)
{
	m_NumofAnim = num;
	m_AnimIndex = 0;
	m_ani = new LoadAnimation*[m_NumofAnim];
	for (UINT i = 0; i < m_NumofAnim; ++i) {
		m_ani[i] = tmp[i];
		m_ani[i]->setBones(m_model->GetBones());
	}
}

void ModelObject::Animate(float fTime)
{
	if (m_ani) {
		m_AnimIndex = AnimIndex;
		m_ani[m_AnimIndex]->BoneTransform(m_AnimIndex, m_Bones);
		AnimIndex = m_AnimIndex;
		//m_Animtime += 0.03f;
		if (physBox) {
			XMFLOAT3 tmp = XMFLOAT3(physBox->getGlobalPose().p.x, physBox->getGlobalPose().p.y, physBox->getGlobalPose().p.z);
			SetPosition(tmp);
		}
		if (tmp) {
			tmp->move(PxVec3(0, 1, 0)*-0.1f, 0.001f, 1, gCharacterControllerFilters);
			XMFLOAT3 tt = XMFLOAT3(tmp->getPosition().x, tmp->getPosition().y, tmp->getPosition().z);
			SetPosition(tt);
		}
		
	}
}

void ModelObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

void ModelObject::SetPosition(XMFLOAT3 pos)
{
	CGameObject::SetPosition(pos.x, pos.y, pos.z);
}

void ModelObject::SetPhysX(PxPhysics * px, PxScene* pscene)
{
	PxCapsuleGeometry tmp(2.0f, 2.0f);
	PxMaterial* mat = px->createMaterial(0.2f, 0.2f, 0.2f);
	PxTransform pos(PxVec3(CGameObject::GetPosition().x, CGameObject::GetPosition().y, CGameObject::GetPosition().z));
	physBox = PxCreateDynamic(*px, pos, tmp, *mat, 1.0f);

	pscene->addActor(*physBox);
}
