#pragma once

#include <vector>
#include <map>

#include "..\Object\Mesh\Mesh.h"
#include "tAnimation.h"

using namespace std;

struct vertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 tex;
	XMFLOAT3 weight;
	XMUINT4	index;
};

class tMesh : public CMesh
{
private:

public:
	tMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		vector<vertex>& vertices, vector<int>& indices);
	virtual ~tMesh();



	//animation
	vector<tBone> m_Bones;
	map<string, Animation> m_Animation;
	Animation* m_currAnimation;

	void StartAnimation(string aName);
	void StopAllAnimation();
	UINT getCurrentAnimPos(XMFLOAT4X4* offset);

	int getNumBone(string aName);
	tBone* getBone(string aName);

	void Tick(float dt);
};