#pragma once

#include "..\Object\Object.h"
#include "LoadModel.h"
#include "Animation.h"

struct CB_OBJECT_INFO
{
	XMFLOAT4X4		m_xmf4x4World;
	XMFLOAT4X4		m_bone[96];
	UINT					m_nMaterial;
};

class ModelObject : public CGameObject
{
private:
	LoadModel* m_model;
	LoadAnimation* m_ani;
	vector<XMFLOAT4X4>	m_Bones;
	float time = 0.0f;
public:
	ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~ModelObject();

	virtual void Animate(float fTime);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual XMFLOAT4X4* GetBoneData() {
		return m_Bones.data();
	}
	virtual int GetBoneNum() const {
		return m_Bones.size();
	}
};