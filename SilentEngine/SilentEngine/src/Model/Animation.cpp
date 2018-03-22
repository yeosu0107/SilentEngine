#include "stdafx.h"
#include "LoadModel.h"
#include "Animation.h"

LoadAnimation::LoadAnimation(string filename) :
	animation_loof(true), next_index(0), m_animSpeed(1.0f)
{
	m_pScene = aiImportFile(filename.c_str(), (aiProcessPreset_TargetRealtime_Quality | aiProcess_ConvertToLeftHanded) & ~aiProcess_FindInvalidData);

	if (m_pScene) {
		m_pAnim = m_pScene->mAnimations[0]; //���� �ִϸ��̼Ǹ� ����ϴ� ��� 0�� �ε���
		//m_GlobalInverse = aiMatrixToXMMatrix(m_pScene->mRootNode->mTransformation);
		m_GlobalInverse = XMMatrixIdentity();

		start_time = (float)m_pAnim->mChannels[0]->mPositionKeys[0].mTime; 
		//������ ���� ������ ��ǥ �̵� �������� �������� ����
		end_time = (float)m_pAnim->mChannels[0]->mPositionKeys[m_pAnim->mChannels[0]->mNumPositionKeys - 1].mTime - 1.0f; 
		//������ ���� �������� 1.0 ��ŭ ����� �������� �Ȱ�ħ
		now_time = start_time;

	}
}

void LoadAnimation::BoneTransform(UINT& index, vector<XMFLOAT4X4>& transforms)
{
	XMMATRIX Identity = XMMatrixIdentity();

	if (!m_pScene) {
		//�ִϸ��̼� ������ �ε�� ���� ��� ����
		for (int i = 0; i < m_NumBones; ++i) {
			XMStoreFloat4x4(&transforms[i], Identity);
		}
		return;
	}

	// �̸� ������ ������ ������ �ִϸ��̼� ����
	now_time += m_animSpeed;
	if (now_time > end_time) {
		now_time = start_time;
		if (!animation_loof) {
			index = next_index;
			return;
		}
	}

	//��Ʈ������ ���������� �m��� ��ȯ ���� �� ���� ������ȯ ���
	ReadNodeHeirarchy(now_time, m_pScene->mRootNode, Identity);

	for (int i = 0; i < m_NumBones; ++i) {
		//���� ������ȯ�� ��ȯ
		XMStoreFloat4x4(&transforms[i], m_Bones[i].second.FinalTransformation);
	}
}

void LoadAnimation::ReadNodeHeirarchy(float AnimationTime, const aiNode * pNode, const XMMATRIX& ParentTransform)
{
	string NodeName(pNode->mName.data);
	const aiAnimation* pAnim = m_pAnim;

	XMMATRIX NodeTransformation = aiMatrixToXMMatrix(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnim, NodeName);
	//���� ��尡 animation channel �� ������ Ȯ��
	if (pNodeAnim) {
		aiVector3D s;
		CalcInterpolatedScaling(s, AnimationTime, pNodeAnim);
		XMMATRIX ScalingM = XMMatrixScaling(s.x, s.y, s.z);


		aiQuaternion q;
		CalcInterpolatedRotation(q, AnimationTime, pNodeAnim);
		XMMATRIX RotationM = XMMatrixRotationQuaternion(XMVectorSet(q.x, q.y, q.z, q.w));


		aiVector3D t;
		CalcInterpolatedPosition(t, AnimationTime, pNodeAnim);
		XMMATRIX TranslationM = XMMatrixTranslation(t.x, t.y, t.z);


		NodeTransformation = ScalingM * RotationM * TranslationM; //������ * ȸ�� * �̵� ��ȯ
		NodeTransformation = XMMatrixTranspose(NodeTransformation);
		//transpos �Ƚ����ָ� �� ����
	}
	//�θ��忡 ��ȯ�� ��ø�ؼ� ���ϱ�
	XMMATRIX GlobalTransformation = ParentTransform * NodeTransformation;

	if (NodeName == "Object010") {
		m_grab = GlobalTransformation;
		//���� ��ȯ ��� ������ ����
	}

	//�����尡 �� ����̸� ��ȯ������ ���� ����
	for (auto& p : m_Bones) {
		if (p.first == NodeName) {
			p.second.FinalTransformation =
				m_GlobalInverse * GlobalTransformation * p.second.BoneOffset;
			break;
		}
	}

	for (UINT i = 0; i < pNode->mNumChildren; ++i) {
		//���������� �̷�. �ڽĳ�� Ž�� �� ��ȯ
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

const aiNodeAnim * LoadAnimation::FindNodeAnim(const aiAnimation * pAnimation, const string NodeName)
{
	for (UINT i = 0; i < pAnimation->mNumChannels; ++i) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (string(pNodeAnim->mNodeName.data) == NodeName)
			return pNodeAnim;
	}

	return nullptr;
}

void LoadAnimation::CalcInterpolatedScaling(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	UINT ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	UINT NextScalingIndex = ScalingIndex + 1;


	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);


	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;

	//assert(Factor >= 0.0f && Factor <= 1.0f);


	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void LoadAnimation::CalcInterpolatedRotation(aiQuaternion & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	UINT RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	UINT NextRotationIndex = (RotationIndex + 1);

	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);


	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;

	//if (Factor > 1.0f)
	//	cout << "E" << endl;
	//assert(Factor >= 0.0f && Factor <= 1.0f);


	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void LoadAnimation::CalcInterpolatedPosition(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	UINT PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	UINT NextPositionIndex = (PositionIndex + 1);


	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);


	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;

	//assert(Factor >= 0.0f && Factor <= 1.0f);

	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

UINT LoadAnimation::FindScaling(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}

UINT LoadAnimation::FindRotation(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}

UINT LoadAnimation::FindPosition(float AnimationTime, const aiNodeAnim * pNodeAnim)
{
	for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}