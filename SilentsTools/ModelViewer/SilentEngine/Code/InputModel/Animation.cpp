#include "stdafx.h"
#include "Animation.h"


KeyFrame::KeyFrame() : 
	timePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{}

KeyFrame::~KeyFrame()
{}

float BoneAnimation::GetStartTime() const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return m_keyFrame.front().timePos;
}

float BoneAnimation::GetEndTime() const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	return m_keyFrame.back().timePos;
}

void BoneAnimation::Interpolate(
	_In_ float t,
	_Out_ DirectX::XMFLOAT4X4& M,
	_In_opt_ bool interpolate) const
{
	if (t <= m_keyFrame.front().timePos)
	{
		XMVECTOR S = XMLoadFloat3(&m_keyFrame.front().Scale);
		XMVECTOR P = XMLoadFloat3(&m_keyFrame.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&m_keyFrame.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= m_keyFrame.back().timePos)
	{
		XMVECTOR S = XMLoadFloat3(&m_keyFrame.back().Scale);
		XMVECTOR P = XMLoadFloat3(&m_keyFrame.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&m_keyFrame.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (UINT i = 0; i < m_keyFrame.size() - 1; ++i)
		{
			if (t >= m_keyFrame[i].timePos && t <= m_keyFrame[i + 1].timePos)
			{
				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

				if (interpolate)
				{
					float lerpPercent = (t - m_keyFrame[i].timePos) / (m_keyFrame[i + 1].timePos - m_keyFrame[i].timePos);

					XMVECTOR s0 = XMLoadFloat3(&m_keyFrame[i].Scale);
					XMVECTOR s1 = XMLoadFloat3(&m_keyFrame[i + 1].Scale);

					XMVECTOR p0 = XMLoadFloat3(&m_keyFrame[i].Translation);
					XMVECTOR p1 = XMLoadFloat3(&m_keyFrame[i + 1].Translation);

					XMVECTOR q0 = XMLoadFloat4(&m_keyFrame[i].RotationQuat);
					XMVECTOR q1 = XMLoadFloat4(&m_keyFrame[i + 1].RotationQuat);

					XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
					XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
					XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

					XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
				}
				else
				{
					XMVECTOR S = XMLoadFloat3(&m_keyFrame[i].Scale);
					XMVECTOR P = XMLoadFloat3(&m_keyFrame[i].Translation);
					XMVECTOR Q = XMLoadFloat4(&m_keyFrame[i].RotationQuat);

					XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
				}

				break;
			}
		}
	}
	AnimationData::Transpose(M);
}

//float AnimationClip::GetClipStartTime() const
//{
//	// Find smallest start time over all bones in this clip.
//	//float t = MathHelper::Infinity;
//	//for(UINT i = 0; i < BoneAnimations.size(); ++i)
//	//{
//	//	t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
//	//}
//
//	//return t;
//}
//
//float AnimationClip::GetClipEndTime() const
//{
//	// Find largest end time over all bones in this clip.
//	//float t = 0.0f;
//	//for(UINT i = 0; i < BoneAnimations.size(); ++i)
//	//{
//	//	t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
//	//}
//
//	//return t;
//}

void AnimationClip::Interpolate(
	_In_ float t,
	_Out_ std::vector<DirectX::XMFLOAT4X4>& boneTransforms,
	_In_opt_ bool interpolate) const
{
	boneTransforms.resize(m_BoneAnimation.size());

	for (UINT i = 0; i < m_BoneAnimation.size(); ++i)
	{
		m_BoneAnimation[i].Interpolate(t, boneTransforms[i], interpolate);
	}
}

float AnimationClip::GetTimeStepAtFrame(_In_ UINT frame) const
{
	return ((Duration / TotalFrames) * frame);
}

UINT AnimationData::BoneCount() const
{
	UINT boneCount = 0;
	for (const auto& animation : m_animations)
	{
		boneCount += static_cast<UINT>(animation.second.m_BoneAnimation.size());
	}
	return boneCount;
}

void AnimationData::GetFinalTransforms(
	//_In_ const std::wstring& clipName,
	_In_ float timePos,
	_In_ std::vector<DirectX::XMFLOAT4X4>& localTransforms,
	_In_opt_ bool interpolate) const
{
	if (m_animations.size() == 0)
	{
		return;
	}

	//UINT numBones = (UINT)_boneOffsets.size();

	//std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = m_animations.find(clipName);
	clip->second.Interpolate(timePos, localTransforms, interpolate);
}

float AnimationData::GetTimeStepAtFrame(
	//_In_ const std::wstring& clipName, 
	_In_ UINT frame) const
{
	if (m_animations.size() == 0)
	{
		return 0.0f;
	}

	auto clip = m_animations.find(clipName);
	return clip->second.GetTimeStepAtFrame(frame);
}