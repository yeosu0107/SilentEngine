#pragma once
#include "D3DUtil.h"
#include "UploadBuffer.h"
#include "Camera.h"
#include "Mesh.h"

class Shaders;

//이동 애니메이션
#define DIR_FORWARD			0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20
//애니메이션 상태
#define ANI_ATTACK				0x40
#define ANI_SKILL					0x80
#define ANI_HITTED				0x100
#define ANI_IDLE					0x120

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY		0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE			0x04
#define RESOURCE_BUFFER				0x05
#define RESOURCE_TEXTURE2D_SHADOWMAP	0x06

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4				m_xmf4x4World = D3DMath::Identity4x4();
	UINT							m_nMaterial = 0;
};

// CB_DYNAMICUI_INFO는 좀 더 생각이 필요한 부분
struct CB_DYNAMICUI_INFO
{
	float							m_nTexturedNum;
	float							m_fTextureScale;
};

struct CB_MAINTEXTURED_INFO
{
	UINT							m_nTexturedNum;
	float							m_fTimeElapsed;
};

struct CB_EFFECT_INFO
{
	UINT							m_nMaxXcount;
	UINT							m_nMaxYcount;
	UINT							m_nNowXcount;
	UINT							m_nNowYcount;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct SRVROOTARGUMENTINFO
{
	UINT							m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	int								m_nTextures = 0;
	vector<ComPtr<ID3D12Resource>>	m_ppd3dTextures;
	vector<ComPtr<ID3D12Resource>>	m_ppd3dTextureUploadBuffers;
	vector<SRVROOTARGUMENTINFO>		m_pRootArgumentInfos;
	vector<UINT>					m_pTextureType;
	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void AddTexture(ID3D12Resource* texture, ID3D12Resource* uploadbuffer, UINT textureType = RESOURCE_TEXTURE2D);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(int index) { return m_pRootArgumentInfos[index].m_d3dSrvGpuDescriptorHandle; }

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const wchar_t *pszFileName, UINT nIndex);
	ComPtr<ID3D12Resource> CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);

	int GetTextureCount() { return(m_nTextures); }
	
	ComPtr<ID3D12Resource> GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	ComPtr<ID3D12Resource> GetUploadBuffer(int nIndex) { return(m_ppd3dTextureUploadBuffers[nIndex]); }

	UINT GetTextureType(int index) { return(m_pTextureType[index]); }

	void ReleaseUploadBuffers();
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4	m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	UINT					m_nReflection = 0;
	unique_ptr<CTexture>	m_pTexture = nullptr;
	Shaders*				m_pShader = nullptr;
		
	void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetReflection(UINT nReflection) { m_nReflection = nReflection; }
	void SetTexture(CTexture *pTexture);
	void SetShader(Shaders *pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class GameObject
{
public:
	GameObject(int nMeshes = 1);
	virtual ~GameObject();

public:
	XMFLOAT4X4													m_xmf4x4World;

	vector<unique_ptr<MeshGeometry>>					m_ppMeshes;
	UINT																m_nMeshes;

	CMaterial*														m_pMaterial = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE					m_d3dCbvGPUDescriptorHandle;

protected:
	unique_ptr<UploadBuffer<CB_GAMEOBJECT_INFO>>   m_pd3dcbGameObject;
	CB_GAMEOBJECT_INFO*									m_pcbMappedGameObject = NULL;

	bool																m_bIsLotate = false;
	bool																m_live = true;
	float																m_moveSpeed = 0.0f;
	bool																m_roofCheck = 0;	//애니메이션 한 루프 종료 여부 (종료시 true)
	int																	m_nRootIndex = 1;
public:
	void SetMesh(int nIndex, MeshGeometry *pMesh);
	void SetShader(Shaders *pShader);
	void SetMaterial(CMaterial *pMaterial);
	void SetLive(bool live) { m_live = live; }

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera = nullptr);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nObject, Camera *pCamera = nullptr);

	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseUploadBuffers();
	virtual void SetRootParameterIndex(const int index) { m_nRootIndex = index; }

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	bool			isLive() const { return m_live; }

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3 xmf3Position);
	void SetLookAt(XMFLOAT3& xmf3Target);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void MoveDir(XMFLOAT3 dir, float fDist = 1.0f);
	virtual bool Move(DWORD dir, float fTime) { return false; }
	virtual bool Move(float fTime) { return false; }
	virtual bool Movement(DWORD input) { return false; }

	virtual void Idle() {}
	virtual void Attack() {}
	virtual void Skill() {}
	virtual void Hitted() {}
	virtual void Death() {}

	bool getAnimRoof() const { return m_roofCheck; }

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);

	virtual XMFLOAT4X4* GetBoneData() { return nullptr; }
	virtual int GetBoneNum() const { return 0; }

	void SetScale(float value);
	void SetSpeed(float speed) { m_moveSpeed = speed; }
};