#include "stdafx.h"
#include "ModelObject.h"

ModelObject::ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : GameObject(model->getNumMesh())
{
	m_model = model;
	m_ani = nullptr;
	m_NumofAnim = 0;
	m_AnimIndex = 0;
	m_Animtime = 0.0f;

	//�Ž� ����
	for (UINT i = 0; i < m_nMeshes; ++i) {
		SetMesh(i, m_model->getMeshes()[i]);
	}

	//�� ���� �ʱ�ȭ
	m_Bones.resize(m_model->GetBones()->size());
	for (auto& p : m_Bones) {
		XMStoreFloat4x4(&p, XMMatrixIdentity());
	}
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
}

void ModelObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	GameObject::Render(pd3dCommandList, pCamera);
}

void ModelObject::SetPosition(XMFLOAT3 pos)
{
	GameObject::SetPosition(pos.x, pos.y, pos.z);
}