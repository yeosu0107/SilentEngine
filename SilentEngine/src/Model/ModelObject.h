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
	bool			mJump;

	void			startJump(PxF32 v0);
	void			stopJump();
	PxF32		getHeight(PxF32 elapsedTime);
};

class ModelObject : public GameObject
{
protected:
	LoadModel *				m_model;			//모델 정보 (vertex, UV and ....)
	LoadAnimation**		m_ani;				//애니메이션 정보 (multi animation)

	vector<XMFLOAT4X4>	m_Bones;			//뼈 정보 (최종 변환 정보)
	float							m_Animtime;		//애니메이션 상대 시간
	UINT							m_AnimIndex;		//현재 애니메이션
	UINT							m_NumofAnim;	//애니메이션 갯수

	float							m_MaxRot = 0.0f;

	XMFLOAT2					m_size;
	//동적인 모델
	PxCapsuleController*	m_Controller;
	PxControllerFilters		m_ControllerFilter;
	//정적인 모델
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

	void ChangeAnimation(UINT nextIndex);

	void stopAnim(bool stop);

	virtual int getAnimIndex() { return m_AnimIndex; }

	//physX 관련 함수
	void SetPhysMesh(BasePhysX* phys, PhysMesh type, string* name = nullptr);
	virtual void SetPhysController(BasePhysX* control, PxUserControllerHitReport* callback, PxExtendedVec3* pos);

	void SetActorPos(float xPos, float yPos, float zPos, float rot);
	void RotationYAxis(float rot);

	void releasePhys();

	void* getControllerActor() { return m_Controller->getActor(); }
};