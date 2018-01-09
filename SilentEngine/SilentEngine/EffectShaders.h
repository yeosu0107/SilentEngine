#pragma once
#include "Shader.h"
#include "Timer.h"

class EffectShaders : public CShader
{
public:
	EffectShaders();
	~EffectShaders();

public:
	virtual void EffectOn(float fExistTime);
	virtual void EffectOff();
	virtual void Update(float fElapsedTime);

protected:
	float m_fElapsedTime;
	float m_fExistTime;
	bool m_bEffectOn;
};

////////////////////////////////////

class FadeEffectShader : public EffectShaders
{
public:
	FadeEffectShader();
	~FadeEffectShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob ** ppd3dShaderBlob);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

public:
	void SetColor(XMFLOAT3& xmf3Color) { 
	m_xmf4FadeColor->x = 1.0f - xmf3Color.x; 
	m_xmf4FadeColor->y = 1.0f - xmf3Color.y;
	m_xmf4FadeColor->z = 1.0f - xmf3Color.z;
	m_xmf4FadeColor->w = m_fElapsedTime;
	 };
	void EffectOn(float fExistTime, bool bFadeIn);

protected:
	XMFLOAT4*						m_xmf4FadeColor;
	XMFLOAT4*						m_xmf4TestFloat4;
	ID3D12Resource					*m_pd3dcbFadeColor = NULL;
	bool							m_bFadeIn;
};