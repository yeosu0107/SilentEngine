#pragma once

#include "..\Object\GameObjects.h"
//#include "..\PhysX\BasePhysX.h"
#include "LoadModel.h"
#include "Animation.h"

struct CB_DYNAMICOBJECT_INFO
{
	XMFLOAT4X4		m_bone[96];
};

class ModelObject : public GameObject
{
private:
	LoadModel * m_model;		//�� ���� (vertex, UV and ....)
	LoadAnimation** m_ani;		//�ִϸ��̼� ���� (multi animation)

	vector<XMFLOAT4X4>	m_Bones;	//�� ���� (���� ��ȯ ����)
	float m_Animtime;				//�ִϸ��̼� ��� �ð�
	UINT m_AnimIndex;				//���� �ִϸ��̼�
	UINT m_NumofAnim;			//�ִϸ��̼� ����

public:
	ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~ModelObject();

	void SetAnimations(UINT num, LoadAnimation** tmp);
	virtual UINT GetNumofAnim() const { return m_NumofAnim; }

	virtual void Animate(float fTime);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera = nullptr);
	
	virtual XMFLOAT4X4* GetBoneData() {
		return m_Bones.data();
	}
	virtual int GetBoneNum() const {
		return m_Bones.size();
	}

	virtual void SetPosition(XMFLOAT3 pos);
};