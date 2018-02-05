#include "stdafx.h"
#include "tAnimation.h"

Animation::Animation() :
	mName("defaultAnimation"),
	mBones(0),
	mBoneRoot(0),
	mIsOn(false),
	mDuration(0.0f),
	mTime(0.0f),
	miFrame(0)
{
}

Animation::Animation(string aName, float aDuration, vector<tBone>* aBones,
	vector<Frame> aFrames) :
	mName(aName),
	mBones(aBones),
	mIsOn(false),
	mDuration(aDuration),
	mTime(0.0f),
	miFrame(0),
	mFrames(aFrames)
{
	for (UINT i = 0; i<mBones->size(); i++)
		if (!mBones->at(i).parent)
			mBoneRoot = &mBones->at(i);
}

void Animation::Start()
{
	mIsOn = true;
}

void Animation::Stop()
{
	mIsOn = false;
}

void Animation::Reset()
{
	mTime = 0.0f;
	miFrame = 0;
}

int Animation::GetNumBone(tBone* aBone)
{
	for (UINT i = 0; i<mBones->size(); ++i)
	{
		if (aBone == &mBones->at(i))
			return i;
	}

	return -1;
}

XMMATRIX Animation::GetInterpolatedOffset(tBone* aBone)
{
	Keyframe* currKey = &mFrames[miFrame].offsets[aBone];
	Keyframe* nextKey = &mFrames[miFrame + 1].offsets[aBone];

	float lerpPercent = (mTime - mFrames[miFrame].time) /
		(mFrames[miFrame + 1].time - mFrames[miFrame].time);

	XMVECTOR t0 = XMLoadFloat3(&currKey->translation);
	XMVECTOR t1 = XMLoadFloat3(&nextKey->translation);
	XMVECTOR s0 = XMLoadFloat3(&currKey->scale);
	XMVECTOR s1 = XMLoadFloat3(&nextKey->scale);
	XMVECTOR r0 = XMLoadFloat4(&currKey->rotationQuat);
	XMVECTOR r1 = XMLoadFloat4(&nextKey->rotationQuat);

	XMVECTOR T = XMVectorLerp(t0, t1, lerpPercent);
	XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
	XMVECTOR R = XMQuaternionSlerp(r0, r1, lerpPercent);

	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	return XMMatrixAffineTransformation(S, zero, R, T);
}

void Animation::UpdateToRoot(tBone* aBone)
{
	XMMATRIX toParentM = XMLoadFloat4x4(&aBone->toParent);
	XMMATRIX frameOffsetM = GetInterpolatedOffset(aBone);

	XMMATRIX toRootM;
	if (aBone->parent)
	{
		XMMATRIX toRootParentM = XMLoadFloat4x4(&aBone->parent->toRoot);
		toRootM = frameOffsetM * toRootParentM;
	}
	else
	{
		toRootM = frameOffsetM;
	}

	XMStoreFloat4x4(&aBone->toRoot, toRootM);

	for (UINT i = 0; i<aBone->childs.size(); i++)
		UpdateToRoot(aBone->childs[i]);
}

void Animation::GetCurrentOffsets(XMFLOAT4X4* aOffsets)
{
	for (UINT i = 0; i<GetNumBones(); i++)
	{
		XMMATRIX offsetM = XMLoadFloat4x4(&mBones->at(i).offset);
		XMMATRIX toRootM = XMLoadFloat4x4(&mBones->at(i).toRoot);

		XMStoreFloat4x4(&aOffsets[i], offsetM * toRootM);
	}
}

void Animation::Tick(float dt)
{
	if (mIsOn)
	{
		mTime += dt * 50.0f;

		if (mTime > mDuration)
		{
			mTime = 0.0f;
			miFrame = 0;
		}

		if (mTime > mFrames[miFrame + 1].time)
			miFrame++;

		UpdateToRoot(mBoneRoot);
	}
}