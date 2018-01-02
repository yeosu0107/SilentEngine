#pragma once

#include "assimp\Importer.hpp"
#include "assimp\cimport.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include <iostream>
#include <vector>
#include <DirectXMath.h>

#pragma comment(lib, "assimp.lib")

using namespace DirectX;
struct vertexData {
	XMFLOAT3 m_pos;
	XMFLOAT2 m_tex;
	XMFLOAT3 m_normal;

	vertexData(XMFLOAT3& pos, XMFLOAT2& tex, XMFLOAT3& normal) {
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
	}
};

class InputModel
{
private:
	std::vector<vertexData> m_Vertices;
public:
	InputModel() {}
	~InputModel() {}

	bool LoadAsset(const std::string& fileName);
	void InitScene(const aiScene* pScene, const struct aiNode* nd);
	void InitMesh(const aiMesh*pMesh);

	void printVertices()
	{
		int num = 0;
		for (const auto& p : m_Vertices) {
			printf("[%d] %f %f %f\n", num++, p.m_pos.x, p.m_pos.y, p.m_pos.z);
		}
	}
};

