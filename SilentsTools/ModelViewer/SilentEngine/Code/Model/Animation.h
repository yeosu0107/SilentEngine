#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

using namespace std;


class LoadAnimation
{
private:
	const aiScene*						m_pScene;	//Model info
	const aiAnimation*					m_pAnim;	//Animation Info
	vector<pair<string, Bone>>		m_Bones;	//Bone Info
	UINT										m_NumBones; //Num of Bones

	XMMATRIX								m_GlobalInverse; //Model Global Matrix

public:
	LoadAnimation(string filename);
	~LoadAnimation() {}

	void setBones(vector<pair<string, Bone>>* bones) {
		m_Bones = *bones;
		m_NumBones = m_Bones.size();
	}

	void BoneTransform(float time, vector<XMFLOAT4X4>& transforms);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
};