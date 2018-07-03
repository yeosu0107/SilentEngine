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
#include <algorithm>
#include "d3dx12.h"
//#include "DDSTextureLoader.h"
#include "DDSTextureLoader12.h"
#include "D3DMath.h"

enum SRVRegisterNumber { 
	SRVTexArray					= 0,
	SRVInstanceData				= 1, 
	SRVTexArrayNorm				= 2, 
	SRVTexture2D				= 3,
	SRVTexture2DNorm			= 4,
	SRVInstanceEffectData		= 5,
	SRVInstanceAnimationInfo	= 6,
	SRVShadowMap				= 7,
	SRVUITextureMap				= 9,
	SRVMultiTexture				= 13,
	SRVFullScreenTexture		= 17,
	SRVFullScreenNormalTexture	= 18
};

enum CBVRegisterNumber {
	CBVPerObject		= 0,
	CBVCameraInfo		= 1,
	CBVObjectInfo		= 2,
	CBVAnimationInfo	= 3,
	CBVMaterial			= 4,
	CBVLights			= 5,
	CBVEffect			= 6,
	CBVFade				= 7,
	CBVFog				= 8,
	CBVUIInfo			= 9
};

enum RTVType {
	RTV_COLOR /* 색상 추출 */,
	RTV_OUTLINENRM /* 외곽선을 위한 노말 벡터 추출 */,
	RTV_NRM, /* 노말 매핑 후의 노말 벡터 추출 */
	RTV_POS
};

extern const int gNumFrameResources;

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

#define FRAME_BUFFER_WIDTH 1280
#define FRAME_BUFFER_HEIGHT 720

#define NUM_DEPTHGBUFFERS 1
#define NUM_RENDERTARGET 3 
#define NUM_HDRBUFFER 1
#define NUM_GBUFFERS NUM_RENDERTARGET + NUM_DEPTHGBUFFERS


const UINT MAX_MAP = 10;

extern UINT	gnCbvSrvDescriptorIncrementSize;

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define NUM_DIRECTION_LIGHTS		1

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

	static ComPtr<ID3D12Resource> CreateTexture2DResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags,
		D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue);

	static ComPtr<ID3D12Resource> CreateTextureResourceFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		const wchar_t *pszFileName,ID3D12Resource* ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates);

	static void CalculateTangentArray(long vertexCount, const XMFLOAT3 *vertex, XMFLOAT3 *normal,
		const XMFLOAT2 *texcoord, long triangleCount, const UINT *indeies, XMFLOAT3 *tangent);
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
//
//struct MeshGeometry 
//{
//	string Name;
//
//	ComPtr<ID3DBlob>		VertexBufferCPU = nullptr;
//	ComPtr<ID3DBlob>		IndexBufferCPU = nullptr;
//
//	ComPtr<ID3D12Resource>	VertexBufferGPU = nullptr;
//	ComPtr<ID3D12Resource>	IndexBufferGPU = nullptr;
//
//	ComPtr<ID3D12Resource>	VertexBufferUploader = nullptr;
//	ComPtr<ID3D12Resource>	IndexBufferUploader = nullptr;
//
//	UINT VertexByteStride = 0;
//	UINT VertexBufferByteSize = 0;
//	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
//	UINT IndexBufferByteSize = 0;
//
//	unordered_map<string, SubmeshGeometry> DrawArgs;
//
//	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
//	{
//		D3D12_VERTEX_BUFFER_VIEW vbv;
//		vbv.BufferLocation	= VertexBufferGPU->GetGPUVirtualAddress();
//		vbv.StrideInBytes	= VertexByteStride;
//		vbv.SizeInBytes		= VertexBufferByteSize;
//		
//		return vbv;
//	}
//
//	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
//	{
//		D3D12_INDEX_BUFFER_VIEW ibv;
//		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
//		ibv.Format = IndexFormat;
//		ibv.SizeInBytes = IndexBufferByteSize;
//
//		return ibv;
//	}
//
//	void DisposeUploaders()
//	{
//		VertexBufferUploader = nullptr;
//		IndexBufferUploader = nullptr;
//	}
//
//};

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


#define MAX_LIGHTS	  4
#define MAX_MATERIALS 8

#define PSO_OBJECT					0
#define PSO_SHADOWMAP				1

#define POINT_LIGHT				1	// 주변광
#define SPOT_LIGHT				2
#define DIRECTIONAL_LIGHT		3

struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;	// 앰비언트 반사 색상 
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;	// 광원의 위치 
	float 					m_fFalloff;		// Direction 조명에서 원의 바깥쪽으로 일어나는 감쇠 효과
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	int					m_bEnable;		// 조명 온오프
	int						m_nType;		// 조명 종류
	float					m_fRange;		// 조명 길이
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
};


#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
	HRESULT hr__ = (x); \
	std::wstring wfn = AnsiToWString(__FILE__); \
	if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif // !ThrowIfFailed

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x) { x->Release(); x = 0; }}
#endif



typedef enum { RotX, RotY, RotZ } RotateAxis;
inline bool IsZero(float fValue) { return fabsf(fValue) < std::numeric_limits<float>::epsilon(); }

namespace Vector3
{
	inline bool IsZero(XMFLOAT3& xmf3Vector)
	{
		if (::IsZero(xmf3Vector.x) && ::IsZero(xmf3Vector.y) && ::IsZero(xmf3Vector.z)) return(true);
		return(false);
	}

	inline XMFLOAT3 XMVectorToFloat3(XMVECTOR& xmvVector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, xmvVector);
		return(xmf3Result);
	}

	inline XMFLOAT3 ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) * fScalar);
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
		return(xmf3Result);
	}

	inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}

	inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
		return(xmf3Result);
	}

	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}

	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2)));
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));

		return(xmf3Result);
	}

	inline float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result.x);
	}

	inline XMFLOAT3 CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result);
	}

	inline XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 m_xmf3Normal;
		XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
		return(m_xmf3Normal);
	}

	inline float Length(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
		return(xmf3Result.x);
	}

	inline float Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2)
	{
		XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
		return(XMConvertToDegrees(XMVectorGetX(xmvAngle)));
	}

	inline float Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
	}

	inline XMFLOAT3 TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
		return(xmf3Result);
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
		return(xmf3Result);
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix)
	{
		return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
	}

	inline XMFLOAT3 Lerp(XMFLOAT3& Vector1, XMFLOAT3& Vector2, const float x) {
		XMFLOAT3 xmf3Result;

		xmf3Result = Add(Vector1, ScalarProduct(Subtract(Vector2, Vector1, false), x, false));

		return xmf3Result;
	}

	inline XMFLOAT3 SubtractAxisZero(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, const RotateAxis eAxis, const bool bNomalize = true)
	{
		XMFLOAT3 xmf3Result;
		XMFLOAT3 xmf3Vector1tmp = xmf3Vector1;
		XMFLOAT3 xmf3Vector2tmp = xmf3Vector2;

		switch (eAxis)
		{
		case RotX:
			xmf3Vector1tmp.x = xmf3Vector2tmp.x = 0.0f;
			break;
		case RotY:
			xmf3Vector1tmp.y = xmf3Vector2tmp.y = 0.0f;
			break;
		case RotZ:
			xmf3Vector1tmp.z = xmf3Vector2tmp.z = 0.0f;
			break;
		}
		if (bNomalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector1tmp) - XMLoadFloat3(&xmf3Vector2tmp)));
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1tmp) - XMLoadFloat3(&xmf3Vector2tmp));

		return(xmf3Result);
	}

	inline float AngleAxisZero(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, const RotateAxis eAxis)
	{
		XMFLOAT3 xmf3Vector1tmp = xmf3Vector1;
		XMFLOAT3 xmf3Vector2tmp = xmf3Vector2;

		switch (eAxis)
		{
		case RotX:
			xmf3Vector1tmp.x = xmf3Vector2tmp.x = 0.0f;
			break;
		case RotY:
			xmf3Vector1tmp.y = xmf3Vector2tmp.y = 0.0f;
			break;
		case RotZ:
			xmf3Vector1tmp.z = xmf3Vector2tmp.z = 0.0f;
			break;
		}

		return Angle(Normalize(xmf3Vector1tmp), Normalize(xmf3Vector2tmp));
	}

	inline float TripleProduct(XMFLOAT3& xmf3Look, XMFLOAT3& xmf3Up, XMFLOAT3& xmf3PlayerToDist)
	{
		return (DotProduct(xmf3Up, CrossProduct(xmf3PlayerToDist, xmf3Look)));
	}
}

namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) + XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}

	inline XMFLOAT4 Multiply(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) * XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}

	inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
		return(xmf4Result);
	}
}

namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL, XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition, XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition), XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 LookToLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3Direction, XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookToLH(XMLoadFloat3(&xmf3EyePosition), XMLoadFloat3(&xmf3Direction), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}

}

namespace Triangle
{
	inline bool Intersect(XMFLOAT3& xmf3RayPosition, XMFLOAT3& xmf3RayDirection, XMFLOAT3& v0, XMFLOAT3& v1, XMFLOAT3& v2, float& fHitDistance)
	{
		return(TriangleTests::Intersects(XMLoadFloat3(&xmf3RayPosition), XMLoadFloat3(&xmf3RayDirection), XMLoadFloat3(&v0), XMLoadFloat3(&v1), XMLoadFloat3(&v2), fHitDistance));
	}
}

namespace Plane
{
	inline XMFLOAT4 Normalize(XMFLOAT4& xmf4Plane)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMPlaneNormalize(XMLoadFloat4(&xmf4Plane)));
		return(xmf4Result);
	}
}
