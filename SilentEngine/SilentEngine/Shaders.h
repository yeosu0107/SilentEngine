#pragma once
#include "D3DUtil.h"
#include "UploadBuffer.h"
#include "D3DMath.h"

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

	virtual void BuildPSO(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1) = 0;

	void CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews) {};
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride) {};
//	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_RootSignature.Get()); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void ReleaseObjects() { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	//virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	ComPtr<ID3D12RootSignature>			m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>		m_CBVHeap = nullptr;
	ComPtr<ID3D12PipelineState>			m_pPSO = nullptr;
	ComPtr<ID3DBlob>					m_VSByteCode = nullptr;
	ComPtr<ID3DBlob>					m_PSByteCode = nullptr;
	vector<D3D12_INPUT_ELEMENT_DESC>	m_InputLayout;
};

class BoxShader : public Shaders
{
public:
	BoxShader();
	~BoxShader();

public:
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState();
	virtual D3D12_BLEND_DESC			CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	void CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews) {};
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride) {};
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

protected:
	unique_ptr<UploadBuffer<ObjectConstants>>	m_ObjectCB = nullptr;
	string										m_sMeshName;
};