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
	LoadModel * m_model;		//모델 정보 (vertex, UV and ....)
	LoadAnimation** m_ani;		//애니메이션 정보 (multi animation)

	vector<XMFLOAT4X4>	m_Bones;	//뼈 정보 (최종 변환 정보)
	float m_Animtime;				//애니메이션 상대 시간
	UINT m_AnimIndex;				//현재 애니메이션
	UINT m_NumofAnim;			//애니메이션 갯수

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