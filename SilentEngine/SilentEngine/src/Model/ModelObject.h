#pragma once

#include "..\Object\GameObjects.h"
#include "..\PhysX\BasePhysX.h"
#include "LoadModel.h"
#include "Animation.h"


struct CB_DYNAMICOBJECT_INFO
{
	XMFLOAT4X4		m_xmf4x4World;
	XMFLOAT4X4		m_bone[96];
	UINT					m_nMaterial = 0;
};

class Jump
{
public:
	Jump();

	PxF32		mV0;
	PxF32		mJumpTime;
	bool		mJump;

	void		startJump(PxF32 v0);
	void		stopJump();
	PxF32		getHeight(PxF32 elapsedTime);
};

class ModelObject : public GameObject
{
protected:
	LoadModel * m_model;		//�� ���� (vertex, UV and ....)
	LoadAnimation** m_ani;		//�ִϸ��̼� ���� (multi animation)

	vector<XMFLOAT4X4>	m_Bones;	//�� ���� (���� ��ȯ ����)
	float m_Animtime;				//�ִϸ��̼� ��� �ð�
	UINT m_AnimIndex;				//���� �ִϸ��̼�
	UINT m_NumofAnim;			//�ִϸ��̼� ����

	//for Dynamic
	PxCapsuleController*	m_Controller;
	PxControllerFilters		m_ControllerFilter;
	//for Static
	PxRigidActor*				m_Actor;
public:
	ModelObject(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~ModelObject();

	virtual void SetAnimations(UINT num, LoadAnimation** tmp);
	virtual UINT GetNumofAnim() const { return m_NumofAnim; }

	virtual void Animate(float fTime);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera = nullptr);
	
	virtual XMFLOAT4X4* GetBoneData() {
		return m_Bones.data();
	}
	virtual int GetBoneNum() const {
		return (int)m_Bones.size();
	}

	//physX ����
	void SetPhysMesh(BasePhysX* phys, PhysMesh type);
	virtual void SetPhysController(BasePhysX* control, PxUserControllerHitReport* callback, PxExtendedVec3* pos);
};