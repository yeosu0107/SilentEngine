#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

class LoadAnimation
{
private:
	const aiScene*						m_pScene;	//모델 정보
	const aiAnimation*					m_pAnim;	//애니메이션 정보
	vector<pair<string, Bone>>		m_Bones;	//뼈 정보
	UINT										m_NumBones; //뼈 갯수

	XMMATRIX								m_GlobalInverse; //모델 글로벌 매트릭스
	XMMATRIX								m_grab; //모델 손 좌표 (for 무기)

	float										start_time; //프레임 시작 시간
	float										end_time;  //프레임 종료 시간
	float										mid_time;	//프레임 중간 시간
	float										now_time;  //현재 프레임
	
	BOOL									animation_loof;  //애니메이션 루프 여부 (기본은 true)
	BOOL									stop_anim = false;
	UINT										next_index;

	float										m_animSpeed;
public:
	LoadAnimation(string filename);
	LoadAnimation(const LoadAnimation& T);
	~LoadAnimation() {}

	void setBones(vector<pair<string, Bone>>* bones) {
		m_Bones = *bones;
		m_NumBones = (UINT)m_Bones.size();
	}
	void DisableLoof(UINT index) {
		animation_loof = false;
		next_index = index;
	}
	void EnableLoof() {
		animation_loof = true;
	}
	void StopAnim(bool stop) { stop_anim = stop; }

	void SetAnimSpeed(float speed) { m_animSpeed *= speed; }

	UINT BoneTransform(UINT& index, float fTime, vector<XMFLOAT4X4>& transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

	void ResetAnimation() { now_time = 0; }
};