#pragma once
#include "D3DUtil.h"
#include "Camera.h"
#include "UploadBuffer.h"
#include "D3DMath.h"
#include "GameObjects.h"

using namespace std;

#define COMPILEDSHADERS CompiledShaders::Instance()
#define ShadowShader ShadowDebugShader::Instance()

struct CB_SCENEBLUR_INFO
{
	XMUINT2 m_BlurScale;
	float	m_Time;
	float	m_Enable;
};

struct CB_HDR_TONEMAPPING_INFO
{
	float m_MiddleGrey;
	float m_LumWhiteSqr;
	float m_EnableHDR;
};


struct CB_HDR_DOWNSCALE_INFO
{
	XMUINT2 m_Res;			// 화면 크기
	UINT	m_Domain;		// 다운 스케일된 이미지 픽셀 수 
	UINT	m_GroupSize;	// 첫 패스에 적용된 그룹 수 
};

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
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout(int index = 0);
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState(int index = 0);
	virtual D3D12_BLEND_DESC			CreateBlendState(int index = 0);
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState(int index = 0);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(int index = 0);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(int index = 0);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(int index = 0);

	void Release() {};

	virtual void BuildPSO(ID3D12Device *pd3dDevice, UINT nRenderTargets = 1, int index = 0) ;
	virtual void BuildComputePSO(ID3D12Device *pd3dDevice, int index = 0);
	void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement);
	void CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement, bool bIsGraphics = true);
	virtual void CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, UINT nPreConstanceBuffers, UINT nElementSize, bool bAutoIncrement);
	void CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceView, bool bIsGraphics = true);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride) ;
//	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature(const UINT index = 0) { return(m_RootSignature[index].Get()); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void ReleaseObjects() { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void DispatchComputePipeline(ID3D12GraphicsCommandList * pd3dCommandList, int index = 0);
	virtual void RenderToDepthBuffer(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int index = 0);
	
	virtual void Animate(float fTimeElapsed) {}
	virtual void CreatePipelineParts();

	virtual void SetMultiUploadBuffer(void** data);

protected:
	ComPtr<ID3D12RootSignature>*					m_RootSignature = nullptr;
	ComPtr<ID3D12RootSignature>*					m_ComputeRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap>					m_CBVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>					m_CbvSrvDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>					m_ComputeCbvSrvDescriptorHeap = nullptr;

	ComPtr<ID3D12PipelineState>*					m_pPSO = nullptr;
	ComPtr<ID3D12PipelineState>*					m_pComputePSO = nullptr;

	ComPtr<ID3DBlob>*								m_VSByteCode = nullptr;
	ComPtr<ID3DBlob>*								m_PSByteCode = nullptr;
	ComPtr<ID3DBlob>*								m_CSByteCode = nullptr;

	vector<D3D12_INPUT_ELEMENT_DESC>				m_pInputElementDesc;
	vector<GameObject* >							m_ppObjects;
	CMaterial										*m_pMaterial = NULL;
	
	UINT											m_nObjects = 0;
	UINT											m_nPSO = 1;
	UINT											m_nComputePSO = 0;
	UINT											m_nDescriptorUAVStartIndex = 0;
	XMUINT3											m_nComputeThreadCount = XMUINT3(1, 1, 1);

	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dComputeCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dComputeCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE						m_d3dComputeSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE						m_d3dComputeSrvGPUDescriptorStartHandle;

	vector<D3D12_INPUT_ELEMENT_DESC>				m_InputLayout;

	UploadBuffer<LIGHTS>*							m_LightsCB = nullptr;
	UploadBuffer<MATERIALS>*						m_MatCB = nullptr;

	DXGI_FORMAT										m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
};

class ObjectShader : public Shaders
{
public:
	ObjectShader() : Shaders() {};
	~ObjectShader() {};

public:
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout(int index = 0);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
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
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int index = 0);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) {};
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

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
		ROOTPARAMETER_TEXTURE
	};
public:
	BillboardShader() {};
	~BillboardShader() {};

	virtual D3D12_BLEND_DESC CreateBlendState(int index = 0);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int index = 0);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);

	virtual void Animate(float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	virtual void SetPos(XMFLOAT3* getPos, UINT num) {}
	virtual void SetLive(const bool live, UINT num) {};
	virtual void SetRotateLockXZ(bool lock);
	virtual void setLoop(bool tmp) {}

	void setAnimSpeed(float speed);
protected:
	Camera * m_pCamera = nullptr;
	float m_fElapsedTime = 0.0f;

	unique_ptr<UploadBuffer<CB_EFFECT_INFO>>	m_EffectCB = nullptr;
};

class TextureToFullScreen : public Shaders
{
public:
	TextureToFullScreen() {};
	virtual ~TextureToFullScreen() {};

public:
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState(int index = 0);
	virtual D3D12_BLEND_DESC CreateBlendState(int index = 0);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
};
class DeferredFullScreen : public Shaders
{
public:
	DeferredFullScreen();
	virtual ~DeferredFullScreen();

	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState(int index = 0);
	virtual D3D12_BLEND_DESC			CreateBlendState(int index = 0);

	void BuildFog();
	void SetNowScene(UINT* nowScene) { m_pNowScene = nowScene; }

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void SetPlayer(GameObject* obj) { m_pPlayer = obj; }
	virtual void Animate(float fTimeElapsed);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
protected:
	unique_ptr<UploadBuffer<CB_SCENEBLUR_INFO>>	m_BulrCB = nullptr;
	unique_ptr<UploadBuffer<CB_FOG_INFO>>				m_FogCB = nullptr;
	unique_ptr<CTexture>				m_pTexture;

	CB_FOG_INFO*										m_pFog;

	GameObject*				m_pPlayer;

	float					m_IsDeath;
	float					m_Time;
	XMUINT2					m_Scale;
	UINT*					m_pNowScene;

protected:
	const unsigned int		MAX_SCALE = 9;
	const unsigned int		BLUR_SPEED = 10;
};

class FadeEffectShader : public DeferredFullScreen
{
public:
	FadeEffectShader();
	virtual ~FadeEffectShader();

	virtual D3D12_BLEND_DESC CreateBlendState(int index = 0);
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState(int index = 0);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Animate(float fTimeElapsed);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList, int index);
	virtual void SetFadeIn(const bool bfadeType = true, const float fExistTime = 1.0f, const bool autoChange = true, const XMFLOAT3& xmf3Color = XMFLOAT3(0.0f, 0.0f, 0.0f));

	virtual bool IsUsdedFadeEffect() const { return m_bFadeOn; }

protected:
	unique_ptr<UploadBuffer<XMFLOAT4>> m_FadeCB;
	XMFLOAT4 m_Color;
	
	bool m_bFadeOn;
	bool m_bFadeType; // true : 페이드 인 , false : 페이드 아웃
	bool m_bAutoChange;	// 자동 페이드 변환
	
	float m_fExistTime;
};

class HDRShader : public DeferredFullScreen
{
public:
	HDRShader() {};
	virtual ~HDRShader() {};

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateComputeRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateUAVResourceView(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT descriptorIndex, UINT nTextureIndex, UINT numElements);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void DispatchComputePipeline(ID3D12GraphicsCommandList * pd3dCommandList, int index = 0);
	virtual void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);

protected:
	unique_ptr<UploadBuffer<CB_HDR_DOWNSCALE_INFO>>		m_HDRDownScaleCB;
	CB_HDR_DOWNSCALE_INFO								m_HDRDownScaleData;

	unique_ptr<UploadBuffer<CB_HDR_TONEMAPPING_INFO>>	m_HDRToneMappCB;
	CB_HDR_TONEMAPPING_INFO								m_HDRToneMappData;

	unique_ptr<CTexture>								m_pComputeTexture;

	ComPtr<ID3D12Resource>								m_pComputeOuput1D;

};

class ShadowDebugShader : public DeferredFullScreen
{
public:
	ShadowDebugShader();
	virtual ~ShadowDebugShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void RefreshShdowMap(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual ID3D12Resource* Rsc(const int index) const { return m_Resource[index].Get(); }
	virtual ID3D12Resource* UploadBuffer(const int index) const { return m_UploadBuffer[index].Get(); }
public:
	static ShadowDebugShader* Instance();

protected:
	ComPtr<ID3D12Resource> m_Resource[NUM_DIRECTION_LIGHTS];
	ComPtr<ID3D12Resource> m_UploadBuffer[NUM_DIRECTION_LIGHTS];
};

class DrawGBuffers : public DeferredFullScreen
{
public:
	DrawGBuffers() { };
	~DrawGBuffers() { };

public:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera);
};