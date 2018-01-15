#pragma once

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include "d3dx12.h"
#include "DDSTextureLoader12.h"
#include "D3DMath.h"

extern const int gNumFrameResources;

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
	if (obj)
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
}

inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
	if (obj)
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
}

inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
	if (obj)
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
}

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class D3DUtil
{
public:

	static bool IsKeyDown(int vkeyCode);

	static UINT CalcConstantBufferByteSize(UINT byteSize) {	return (byteSize + 255) & ~255; }
	
	static ComPtr<ID3DBlob> LoadBinary(const wstring& filename);
	
	static ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer);
	
	static ComPtr<ID3DBlob> CompileShader(
		const wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const string& entrypoint,
		const string& target);

};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const wstring& functionName, const wstring& filename, int lineNumber);

public:
	wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	wstring FunctionName;
	wstring Filename;

	int LineNumber = -1;
};

struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
};

struct MeshGeometry 
{
	string Name;

	ComPtr<ID3DBlob>		VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob>		IndexBufferCPU = nullptr;

	ComPtr<ID3D12Resource>	VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource>	IndexBufferGPU = nullptr;

	ComPtr<ID3D12Resource>	VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource>	IndexBufferUploader = nullptr;

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	unordered_map<string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation	= VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes	= VertexByteStride;
		vbv.SizeInBytes		= VertexBufferByteSize;
		
		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}

};

struct Light
{
	XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;
	XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
	float FalloffEnd = 10.0f;
	XMFLOAT3 Position = { 0.0f,0.0f,0.0f };
	float SpotPower = 64.0f;
};

#define MaxLights 16

struct MaterialConstants
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = D3DMath::Identity4x4();
};

struct Material
{
	string Name;

	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	int NumFramesDirty = gNumFrameResources;

	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	XMFLOAT4X4 MatTransform = D3DMath::Identity4x4();
};

struct Texture
{
	string Name;
	wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
	HRESULT hr__ = (x); \
	std::wstring wfn = AnsiToWString(__FILE__); \
	if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif // !ThrowIfFailed
