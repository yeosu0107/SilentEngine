#pragma once
#include <map>
using namespace std;

struct KeyFrame
{
	KeyFrame();
	~KeyFrame();

	float timePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(
		_In_ float t,
		_Out_ XMFLOAT4X4& m,
		_In_opt_ bool interpolate = true) const;

	vector<KeyFrame> m_keyFrame;
	string name;
};

struct AnimationClip
{
	float GetTimeStepAtFrame(_In_ UINT frame) const;

	void Interpolate(
		_In_ float t,
		_Out_ vector<XMFLOAT4X4>& boneTransforms,
		_In_opt_ bool interpolate = true) const;

	vector<BoneAnimation> m_BoneAnimation;
	float TicksPerSecond;
	float Duration;
	string name;
	UINT TotalFrames;
};

class AnimationData
{
public:
	UINT BoneCount() const;

	inline static void Transpose(_In_ DirectX::XMFLOAT4X4& m)
	{
		DirectX::XMMATRIX M = XMLoadFloat4x4(&m);

		M = XMMatrixTranspose(M);

		XMStoreFloat4x4(&m, M);
	}

	bool ContainsData() const
	{
		return m_animations.size() > 0;
	}

	float GetTimeStepAtFrame(_In_ UINT frame) const;

	void AddAnimationClip(_In_ AnimationClip animationClip)
	{
		animationClip.TotalFrames = static_cast<UINT>(animationClip.Duration * 30.0f);
		m_animations[animationClip.name] = animationClip;
		clipName = animationClip.name;
	}

	void SetCurrentClip(_In_ string clipNames)
	{
		clipName = clipNames;
	}

	void GetFinalTransforms(
		_In_ float timePos,
		_In_ std::vector<DirectX::XMFLOAT4X4>& localTransforms,
		_In_opt_ bool interpolate = true) const;

	map<string, AnimationClip> m_animations;

	string clipName;

private:

	
};


