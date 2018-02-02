#pragma once
#include "Mesh.h"
#include "D3DUtil.h"

class GlobalObjects
{
public:
	GlobalObjects();
	~GlobalObjects();

public:
	void BuildGlobalMeshes();
	void BuildShaders();
	void BuildPSOs();

public:
	static unordered_map<string, unique_ptr<MeshGeometry>> GlobalMesh;
	static unordered_map<string, ComPtr<ID3DBlob>> GlobalShaders;
	static unordered_map<string, ComPtr<ID3D12PipelineState>> GlobalPSOs;
};

