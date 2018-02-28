#include "stdafx.h"
#include "Camera.h"
#include "Shaders.h"

CompiledShaders::CompiledShaders()
{
	CompiledShader = make_unique<unordered_map<string, ComPtr<ID3DBlob>>>();
}

ComPtr<ID3DBlob> CompiledShaders::GetCompiledShader(const wstring & filename, const D3D_SHADER_MACRO * defines, const string & entrypoint, const string & target)
{
	if((*CompiledShader)[entrypoint] == nullptr)
		(*CompiledShader)[entrypoint] = D3DUtil::CompileShader(filename, defines, entrypoint, target);
	
	return (*CompiledShader)[entrypoint];
}



void Shaders::BuildPSO(ID3D12Device * pd3dDevice, ID3D12RootSignature * pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	::ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	
	psoDesc.InputLayout				= CreateInputLayout();
	psoDesc.pRootSignature			= m_RootSignature.Get();
	psoDesc.VS						= CreateVertexShader();
	psoDesc.PS						= CreatePixelShader();
	psoDesc.RasterizerState			= CreateRasterizerState();
	psoDesc.BlendState				= CreateBlendState();
	psoDesc.DepthStencilState		= CreateDepthStencilState();
	psoDesc.SampleMask				= UINT_MAX;
	psoDesc.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets		= nRenderTargets;
	psoDesc.RTVFormats[0]			= DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count		= 1;
	psoDesc.DSVFormat				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	ThrowIfFailed(pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO)));
}
void Shaders::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
}

D3D12_INPUT_LAYOUT_DESC Shaders::CreateInputLayout()
{
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;
	D3D12_INPUT_LAYOUT_DESC			 inputLayout;

	inputElementDesc =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	inputLayout = { inputElementDesc.data(), (UINT)inputElementDesc.size() };

	return inputLayout;
}

D3D12_RASTERIZER_DESC Shaders::CreateRasterizerState()
{
	return CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
}

D3D12_BLEND_DESC Shaders::CreateBlendState()
{
	return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
}

D3D12_DEPTH_STENCIL_DESC Shaders::CreateDepthStencilState()
{
	return CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

D3D12_SHADER_BYTECODE Shaders::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE byteCode;
	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));
	byteCode.pShaderBytecode = m_VSByteCode->GetBufferPointer();
	byteCode.BytecodeLength = m_VSByteCode->GetBufferSize();

	return byteCode;
}

D3D12_SHADER_BYTECODE Shaders::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE byteCode;
	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));
	byteCode.pShaderBytecode = m_PSByteCode->GetBufferPointer();
	byteCode.BytecodeLength = m_PSByteCode->GetBufferSize();

	return byteCode;
}

void Shaders::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
}

void Shaders::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
}

void Shaders::ReleaseShaderVariables()
{
}

void Shaders::UpdateShaderVariable(ID3D12GraphicsCommandList * pd3dCommandList, XMFLOAT4X4 * pxmf4x4World)
{
}

void Shaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_VSByteCode = g_CompiledShaders.GetCompiledShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_PSByteCode = g_CompiledShaders.GetCompiledShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}

void Shaders::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

}

void Shaders::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList)
{
}

//////////////////////////////////////////////////////////////////////////////
//
//D3D12_INPUT_LAYOUT_DESC BoxShader::CreateInputLayout()
//{
//	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;
//	D3D12_INPUT_LAYOUT_DESC			 inputLayout;
//
//	inputElementDesc =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//	};
//	inputLayout = { inputElementDesc.data(), (UINT)inputElementDesc.size() };
//
//	return inputLayout;
//}
//
//D3D12_RASTERIZER_DESC BoxShader::CreateRasterizerState()
//{
//	return CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
//}
//
//D3D12_BLEND_DESC BoxShader::CreateBlendState()
//{
//	return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//}
//
//D3D12_DEPTH_STENCIL_DESC BoxShader::CreateDepthStencilState()
//{
//	return CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
//}
//
//D3D12_SHADER_BYTECODE BoxShader::CreateVertexShader()
//{
//	D3D12_SHADER_BYTECODE byteCode;
//	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));
//	byteCode.pShaderBytecode = m_VSByteCode->GetBufferPointer();
//	byteCode.BytecodeLength = m_VSByteCode->GetBufferSize();
//
//	return byteCode;
//}
//
//D3D12_SHADER_BYTECODE BoxShader::CreatePixelShader()
//{
//	D3D12_SHADER_BYTECODE byteCode;
//	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));
//	byteCode.pShaderBytecode = m_PSByteCode->GetBufferPointer();
//	byteCode.BytecodeLength = m_PSByteCode->GetBufferSize();
//
//	return byteCode;
//}
//
//void BoxShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
//{
//	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
//
//	CD3DX12_DESCRIPTOR_RANGE cbvTable;
//	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
//	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);
//
//	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
//		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
//
//	ComPtr<ID3DBlob> serializedRootSig = nullptr;
//	ComPtr<ID3DBlob> errorBlob = nullptr;
//	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
//		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
//
//	if (errorBlob != nullptr)
//	{
//		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//	}
//	ThrowIfFailed(hr);
//
//	ThrowIfFailed(pd3dDevice->CreateRootSignature(
//		0,
//		serializedRootSig->GetBufferPointer(),
//		serializedRootSig->GetBufferSize(),
//		IID_PPV_ARGS(&m_RootSignature)));
//}

