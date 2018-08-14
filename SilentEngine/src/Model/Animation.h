#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

class LoadAnimation
{
private:
	const aiScene*						m_pScene;	//�� ����
	const aiAnimation*					m_pAnim;	//�ִϸ��̼� ����
	vector<pair<string, Bone>>		m_Bones;	//�� ����
	UINT										m_NumBones; //�� ����

	XMMATRIX								m_GlobalInverse; //�� �۷ι� ��Ʈ����
	XMMATRIX								m_grab; //�� �� ��ǥ (for ����)

	float										start_time; //������ ���� �ð�
	float										end_time;  //������ ���� �ð�
	float										trigger_time;	//������ �߰� �ð�
	float										now_time;  //���� ������
	float										posible_skip; //�ִϸ��̼��� ���� �����ϰ� ���� �ִϸ��̼� ���� ������ ������
	
	BOOL									animation_loof;  //�ִϸ��̼� ���� ���� (�⺻�� true)
	BOOL									stop_anim = false;
	UINT										next_index;

	float										m_animSpeed;
	float										m_prevSpeed;
public:
	LoadAnimation(string filename, float trigger, float skip);
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
	void loadAnimSpeed() { m_animSpeed = m_prevSpeed; }
	void saveAnimSpeed() { m_prevSpeed = m_animSpeed; }

	UINT BoneTransform(UINT& index, float fTime, vector<XMFLOAT4X4>& transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string& NodeName);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

	void ResetAnimation() { now_time = 0; }
	void SetAnimFrame(float frame);

	float getAnimTime() const { return now_time; }
};