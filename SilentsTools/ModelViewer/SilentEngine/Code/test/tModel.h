#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include <map>
#include "tMesh.h"

//#pragma comment(lib, "assimp.lib")
using namespace std;

class tModel
{
private:
	string name;
	tMesh* m_mesh;
	vector<tBone> m_bones;
public:
	tModel();
	~tModel();
	bool LoadFile(const char* fileName);
	bool LoadBones(const aiScene* pScene, aiMesh* pMesh,
		vector<vertex>* vertices);

	bool LoadAnimation(const aiScene* pScene);
};