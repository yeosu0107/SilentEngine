#pragma once
#include <map>
using namespace std;

struct tBone
{
	string name;
	XMFLOAT4X4 offset;
	XMFLOAT4X4 toParent;

	tBone* parent;
	vector<tBone*> childs;

	XMFLOAT4X4 toRoot;
};

struct Keyframe
{
	XMFLOAT3 translation;
	XMFLOAT3 scale;
	XMFLOAT4 rotationQuat;
};

struct Frame
{
	float time;
	map<tBone*, Keyframe> offsets;
};

class Animation
{
private:
	string mName;
	tBone* mBoneRoot;
	vector<tBone>* mBones;

	bool  mIsOn;
	float mDuration;
	float mTime;
	UINT  miFrame;
	vector<Frame> mFrames;

	int GetNumBone(tBone* aBone);
	void UpdateToRoot(tBone* aBone);
	XMMATRIX GetInterpolatedOffset(tBone* aBone);
public:
	Animation();
	Animation(string aName, float aDuration, vector<tBone>* aBones,
		vector<Frame> aFrames);

	bool IsOn() { return mIsOn; };

	void Start();
	void Stop();
	void Reset();

	void GetCurrentOffsets(XMFLOAT4X4* aOffsets);
	UINT GetNumBones() { return mBones->size(); };

	void Tick(float dt);
};