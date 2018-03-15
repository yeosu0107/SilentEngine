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
	float										now_time;  //���� ������
	BOOL									animation_loof;  //�ִϸ��̼� ���� ���� (�⺻�� true)
	UINT										next_index;
public:
	LoadAnimation(string filename);
	~LoadAnimation() {}

	void setBones(vector<pair<string, Bone>>* bones) {
		m_Bones = *bones;
		m_NumBones = m_Bones.size();
	}
	void DisableLoof(UINT index) {
		animation_loof = false;
		next_index = index;
	}
	void EnableLoof() {
		animation_loof = true;
	}

	void BoneTransform(UINT& index, vector<XMFLOAT4X4>& transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
};