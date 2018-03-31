#pragma once
#include "D3DUtil.h"
#include "Camera.h"
#include "UploadBuffer.h"
#include "D3DMath.h"
#include "GameObjects.h"

using namespace std;

#define COMPILEDSHADERS CompiledShaders::Instance()

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType);
class CompiledShaders
{
public:
	CompiledShaders();
	~CompiledShaders() {};

public:
	unordered_map<string, ComPtr<ID3DBlob>> CompiledShader;
	
	ComPtr<ID3DBlob> GetCompiledShader(const wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const string& entrypoint,
		const string& target);

	static CompiledShaders* Instance();
};

class Shaders
{
public:
	Shaders() {};
	virtual ~Shaders() {};

public:
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState();
	virtual D3D12_BLEND_DESC			CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void Release() {};

	virtual void BuildPSO(ID3D12Device *pd3dDevice, UINT nRenderTargets = 1) ;
	virtual void BuildPSO(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature, UINT nRenderTargets = 1);
	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement);
	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);
	virtual void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, UINT nPreConstanceBuffers, UINT nElementSize, bool bAutoIncrement);
	void CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride) ;
//	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_RootSignature.Get()); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	
	virtual void Animate(float fTimeElapsed) {}
protected:
	ComPtr<ID3D12RootSignature>						m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>					m_CBVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>					m_CbvSrvDescriptorHeap = nullptr;
	ComPtr<ID3D12PipelineState>						m_pPSO = nullptr;
	ComPtr<ID3DBlob>								m_VSByteCode = nullptr;
	ComPtr<ID3DBlob>								m_PSByteCode = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC>				m_pInputElementDesc;
	vector<GameObject* >							m_ppObjects;
	CMaterial										*m_pMaterial = NULL;
	UINT											m_nObjects = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dSrvGPUDescriptorStartHandle;

	vector<D3D12_INPUT_ELEMENT_DESC>				m_InputLayout;
};

class ObjectShader : public Shaders
{
public:
	ObjectShader() : Shaders() {};
	~ObjectShader() {};

public:
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

protected:
	unique_ptr<UploadBuffer<CB_GAMEOBJECT_INFO>>	m_ObjectCB = nullptr;
};

class NormalMapShader : public ObjectShader
{
public:
	NormalMapShader();
	~NormalMapShader();

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) {};
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	virtual void SetLightsUploadBuffer(UploadBuffer<LIGHTS>* pLightBuf) { m_LightsCB = pLightBuf; }
	virtual void SetMaterialUploadBuffer(UploadBuffer<MATERIALS>* pMatBuf) { m_MatCB = pMatBuf; }

protected:
	UploadBuffer<LIGHTS>*							m_LightsCB = nullptr;
	UploadBuffer<MATERIALS>*						m_MatCB = nullptr;
};

class BillboardShader : public NormalMapShader
{
protected:
	enum { 
		ROOTPARAMETER_CAMERA, 
		ROOTPARAMETER_OBJECT, 
		ROOTPARAMETER_MATERIAL, 
		ROOTPARAMETER_LIGHTS, 
		ROOTPARAMETER_EFFECT, 
		ROOTPARAMETER_TEXTURE, 
		ROOTPARAMETER_NORMALMAP 
	};
public:
	BillboardShader() {};
	~BillboardShader() {};

	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void Animate(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; };

protected:
	Camera * m_pCamera = nullptr;
	float m_fElapsedTime = 0.0f;

	unique_ptr<UploadBuffer<CB_EFFECT_INFO>>	m_EffectCB = nullptr;

	float  m_fAnimationSpeed = 10.0f;
	float	m_fMaxXCount = 0.0f;
	float	m_fNowXCount = 0.0f;
	float	m_fMaxYCount = 0.0f;
	float	m_fNowYCount = 0.0f;
};

class TextureToFullScreen : public Shaders
{
public:
	TextureToFullScreen();
	virtual ~TextureToFullScreen();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

protected:
	unique_ptr<CTexture> m_pTexture;
};