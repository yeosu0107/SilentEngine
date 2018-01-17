#include "stdafx.h"
#include "EffectShaders.h"


EffectShaders::EffectShaders()
{
	m_bEffectOn = false;
	m_fExistTime = 1.0f;
}

EffectShaders::~EffectShaders()
{
}

void EffectShaders::EffectOn(float fExistTime)
{
	m_fExistTime = fExistTime;
	m_bEffectOn = true;
}

void EffectShaders::EffectOff()
{
	m_bEffectOn = false;
}

void EffectShaders::Update(float fElapsedTime)
{
	if (!m_bEffectOn) return;

	m_fElapsedTime += fElapsedTime;

	if (m_fElapsedTime > m_fExistTime)
		m_bEffectOn = false;
}

/////////////////////////////////////////////////

FadeEffectShader::FadeEffectShader()
{
	m_xmf4FadeColor = new XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_bFadeIn = false;
}

FadeEffectShader::~FadeEffectShader()
{
}

D3D12_DEPTH_STENCIL_DESC FadeEffectShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return d3dDepthStencilDesc;
}

D3D12_BLEND_DESC FadeEffectShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

D3D12_SHADER_BYTECODE FadeEffectShader::CreatePixelShader(ID3DBlob ** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"./hlsl/Effects.hlsl", "PSFadeeffect", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE FadeEffectShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"./hlsl/Effects.hlsl", "VSFadeeffect", "vs_5_1", ppd3dShaderBlob));
}

void FadeEffectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	UINT ncbElementBytes = ((sizeof(XMFLOAT4) + 255) & ~255); //256의 배수

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 0);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbFadeColor, ncbElementBytes);
}

void FadeEffectShader::CreateShader(ID3D12Device * pd3dDevice, ID3D12RootSignature * pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void FadeEffectShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 7;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

}

void FadeEffectShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(XMFLOAT4) + 255) & ~255); //256의 배수
	m_pd3dcbFadeColor = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbFadeColor->Map(0, NULL, reinterpret_cast<void**>(&m_mappedxmf44FadeColor));
}

void FadeEffectShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(XMFLOAT4) + 255) & ~255);

	m_xmf4FadeColor->w = m_fElapsedTime / m_fExistTime;

	XMFLOAT4* pbMappedFadeColor = m_mappedxmf44FadeColor;
	memcpy(pbMappedFadeColor, m_xmf4FadeColor, ncbElementBytes);

}

void FadeEffectShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera)
{
	if (!m_bEffectOn) 
		return;

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);	// 루트 시그니쳐를 Set한뒤

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);			// 파이프 라인을 셋한 뒤

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbFadeColorGpuVirtualAddress = m_pd3dcbFadeColor->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbFadeColorGpuVirtualAddress); //Materials

	UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void FadeEffectShader::ReleaseUploadBuffers()
{
}

void FadeEffectShader::ReleaseShaderVariables()
{
	if (m_pd3dcbFadeColor)
	{
		m_pd3dcbFadeColor->Unmap(0, NULL);
		m_pd3dcbFadeColor->Release();
	}
}

void FadeEffectShader::EffectOn(float fExistTime, bool bFadeIn)
{
	m_bFadeIn = bFadeIn;
	m_fExistTime = fExistTime;
	m_fElapsedTime = m_bFadeIn ? fExistTime : 0.0f;
	m_bEffectOn = true;
}

void FadeEffectShader::Update(float fElapsedTime)
{
	if (!m_bEffectOn) return;

	if (m_bFadeIn)
		m_fElapsedTime -= fElapsedTime;
	else
		m_fElapsedTime += fElapsedTime;

	if (m_fElapsedTime > m_fExistTime)
		m_bEffectOn = false;
}