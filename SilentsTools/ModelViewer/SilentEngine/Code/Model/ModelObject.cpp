#include "stdafx.h"
#include "ModelObject.h"

ModelObject::ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) : CGameObject(model->getNumMesh())
{
	m_model = model;
	
	for (UINT i = 0; i < m_nMeshes; ++i) {
		SetMesh(i, m_model->getMeshes()[i]);
	}
	m_ani = new LoadAnimation("angle.FBX");
	m_ani->setBones(m_model->GetBones());
	
	//Rotate(90 ,0, 0);
}
ModelObject::~ModelObject() 
{
	CGameObject::~CGameObject();
}

void ModelObject::Animate(float fTime)
{
	m_ani->BoneTransform(time, m_Bones);
	time += 0.01f;
}

void ModelObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}


