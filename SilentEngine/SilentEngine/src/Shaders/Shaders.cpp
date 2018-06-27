#include "stdafx.h"
#include "Camera.h"
#include "Shaders.h"
#include "InstanceObject.h"

CompiledShaders::CompiledShaders()
{
}

ComPtr<ID3DBlob> CompiledShaders::GetCompiledShader(const wstring & filename, const D3D_SHADER_MACRO * defines, const string & entrypoint, const string & target)
{
	if(CompiledShader[entrypoint] == nullptr)
		CompiledShader[entrypoint] = D3DUtil::CompileShader(filename, defines, entrypoint, target);
	
	return CompiledShader[entrypoint];
}

CompiledShaders * CompiledShaders::Instance()
{
	static CompiledShaders instance;

	return &instance;
}

void Shaders::BuildPSO(ID3D12Device * pd3dDevice, UINT nRenderTargets, int index)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	::ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	
	psoDesc.InputLayout				= CreateInputLayout(index);
	psoDesc.pRootSignature			= m_RootSignature[index].Get();
	psoDesc.VS						= CreateVertexShader(index);
	psoDesc.PS						= CreatePixelShader(index);
	psoDesc.RasterizerState			= CreateRasterizerState(index);
	psoDesc.BlendState				= CreateBlendState(index);
	psoDesc.DepthStencilState		= CreateDepthStencilState(index);
	psoDesc.SampleMask				= UINT_MAX;
	psoDesc.PrimitiveTopologyType	= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets		= nRenderTargets;
	for (UINT i = 0; i < nRenderTargets; i++) 
		psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count		= 1;
	psoDesc.DSVFormat				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	ThrowIfFailed(pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO[index].GetAddressOf())));
}

void Shaders::CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;
	int nTextures = pTexture->GetTextureCount();
	
	//nInstanceParameterCount = Index앞에 존재하는 인스턴스 SRV개수

	d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;
	d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;

	for (int i = 0; i < nTextures; i++)
	{
		int nTextureType = pTexture->GetTextureType(i);

		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetTexture(i);
		D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
		d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, d3dSrvGPUDescriptorHandle);
		d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}

}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	case RESOURCE_TEXTURE2D_SHADOWMAP:
		d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d3dShaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

void Shaders::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;
	int nTextures = pTexture->GetTextureCount();
	
	for (int i = 0; i < nTextures; i++)
	{
		int nTextureType = pTexture->GetTextureType(i);
		ComPtr<ID3D12Resource> pShaderResource = pTexture->GetTexture(i);
		D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
		pd3dDevice->CreateShaderResourceView(pShaderResource.Get(), &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
		d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameterStartIndex + i) : nRootParameterStartIndex, d3dSrvGPUDescriptorHandle);
		d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
}

void Shaders::CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource * pd3dConstantBuffers, UINT nRootParameterStartIndex, UINT nPreInstanceBuffers, UINT nElementSize, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;
	/*
		nPreInstanceBuffers = 이전에 선언한 인스턴스 버퍼의 개수
			ex) 
			3개의 인스턴스 버퍼가 있다는 가정하에
			1. nPreInstanceBuffers = 0
			2. nPreInstanceBuffers = 1
			3. nPreInstanceBuffers = 2
	*/
	d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nPreInstanceBuffers;
	d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nPreInstanceBuffers;

	for (int i = 0; i < 1; i++)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dConstantBuffers->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, RESOURCE_BUFFER);

		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_nObjects;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = nElementSize;

		pd3dDevice->CreateShaderResourceView(pd3dConstantBuffers, &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
	}
}

void Shaders::CreateCbvAndSrvDescriptorHeaps(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_CbvSrvDescriptorHeap);
	
	ThrowIfFailed(pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		IID_PPV_ARGS(&m_CbvSrvDescriptorHeap)));

	m_d3dCbvCPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

void Shaders::CreateConstantBufferViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nConstantBufferViews, ID3D12Resource * pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; ++j)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void Shaders::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
}

D3D12_INPUT_LAYOUT_DESC Shaders::CreateInputLayout(int index)
{
	return D3D12_INPUT_LAYOUT_DESC();
}

D3D12_RASTERIZER_DESC Shaders::CreateRasterizerState(int index)
{
	CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	if (index == PSO_SHADOWMAP) {
		rasterizerDesc.DepthBias = 100000;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	}
	return rasterizerDesc;
}

D3D12_BLEND_DESC Shaders::CreateBlendState(int index)
{
	return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
}

D3D12_DEPTH_STENCIL_DESC Shaders::CreateDepthStencilState(int index)
{
	return CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

D3D12_SHADER_BYTECODE Shaders::CreateVertexShader(int index)
{
	D3D12_SHADER_BYTECODE byteCode;
	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));
	
	if (m_VSByteCode[index] != nullptr) {
		byteCode.pShaderBytecode = m_VSByteCode[index]->GetBufferPointer();
		byteCode.BytecodeLength = m_VSByteCode[index]->GetBufferSize();
	}
	return byteCode;
}

D3D12_SHADER_BYTECODE Shaders::CreatePixelShader(int index)
{
	D3D12_SHADER_BYTECODE byteCode;
	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));

	if (m_PSByteCode[index] != nullptr) {
		byteCode.pShaderBytecode = m_PSByteCode[index]->GetBufferPointer();
		byteCode.BytecodeLength = m_PSByteCode[index]->GetBufferSize();
	}

	return byteCode;
}

void Shaders::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList) {}

void Shaders::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList) {}

void Shaders::ReleaseShaderVariables() {}

void Shaders::UpdateShaderVariable(ID3D12GraphicsCommandList * pd3dCommandList, XMFLOAT4X4 * pxmf4x4World) {}

void Shaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext){}


void Shaders::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	OnPrepareRender(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	//pCamera->UpdateShaderVariables(pd3dCommandList);
}

void Shaders::RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	OnPrepareRender(pd3dCommandList, PSO_SHADOWMAP);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	//pCamera->UpdateShaderVariables(pd3dCommandList);
}

void Shaders::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	if (m_RootSignature[index])
		pd3dCommandList->SetGraphicsRootSignature(m_RootSignature[index].Get());

	if (m_pPSO[index])
		pd3dCommandList->SetPipelineState(m_pPSO[index].Get());

	pd3dCommandList->SetDescriptorHeaps(1, m_CbvSrvDescriptorHeap.GetAddressOf());

	UpdateShaderVariables(pd3dCommandList);
}
void Shaders::CreatePipelineParts()
{
	m_pPSO = new ComPtr<ID3D12PipelineState>[m_nPSO];
	m_RootSignature = new ComPtr<ID3D12RootSignature>[m_nPSO];

	m_VSByteCode = new ComPtr<ID3DBlob>[m_nPSO];
	m_PSByteCode = new ComPtr<ID3DBlob>[m_nPSO];
}

void Shaders::SetMultiUploadBuffer(void** data)
{
	m_LightsCB	= reinterpret_cast<UploadBuffer<LIGHTS>*>(data[0]);
	m_MatCB		= reinterpret_cast<UploadBuffer<MATERIALS>*>(data[1]);
	m_FogCB		= reinterpret_cast<UploadBuffer<CB_FOG_INFO>*>(data[2]);
}
//
//// /////////////////////////////////////////////////

D3D12_INPUT_LAYOUT_DESC ObjectShader::CreateInputLayout(int index)
{
	//vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	inputLayout = { m_pInputElementDesc.data(), (UINT)m_pInputElementDesc.size() };

	return inputLayout;
}

void ObjectShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, SRVTexArrayNorm, 0, 0); // GameObject
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArray, 0, 0); // Texture

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);
		
	d3dSamplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
	);


	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void ObjectShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, true);

	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_GAMEOBJECT_INFO));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();
}

void ObjectShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);
	}
}

void ObjectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
}

void ObjectShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	for (unsigned int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

//////////////////////////////////////////////////////////////////////

NormalMapShader::NormalMapShader() { }

NormalMapShader::~NormalMapShader(){ }

D3D12_INPUT_LAYOUT_DESC NormalMapShader::CreateInputLayout(int index)
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT	, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	inputLayout = { m_pInputElementDesc.data(), (UINT)m_pInputElementDesc.size() };

	return inputLayout;
}

void NormalMapShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 1;
	m_nPSO = 1;
	
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\NormalMap.hlsl", nullptr, "VSNormalMap", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSMultirender", "ps_5_0");

	CTexture *pTexture = new CTexture(2, RESOURCE_TEXTURE2DARRAY, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\bricks.dds", 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\bricks_nm.dds", 1);
	for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_GAMEOBJECT_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	NormalMappingCube *pCubeMesh = new NormalMappingCube(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 50.0f);

	m_ppObjects = vector<GameObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i] = new GameObject();
		m_ppObjects[i]->SetMesh(0, pCubeMesh);
		m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}

	
}

void NormalMapShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, true);
}

void NormalMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(2, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, m_LightsCB->Resource()->GetGPUVirtualAddress());
}

void NormalMapShader::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList)
{
	Shaders::OnPrepareRender(pd3dCommandList);
}

void NormalMapShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArray, 0, 0); // Texture
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArrayNorm, 0, 0); // Texture

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[5];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsConstantBufferView(4);
	pd3dRootParameters[3].InitAsConstantBufferView(5);
	pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	
	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	d3dSamplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
	);



	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

/////////////////////////////////////


void BillboardShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{

	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;
	int i = 0;
	
	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3 + NUM_DIRECTION_LIGHTS];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceData, 0, 0);
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexture2D, 0, 0);
	pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceEffectData, 0, 0);
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dDescriptorRanges[3 + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVShadowMap + i, 0, 0);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[7 + NUM_DIRECTION_LIGHTS];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsConstantBufferView(4);
	pd3dRootParameters[3].InitAsConstantBufferView(5);
	pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[5].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[6].InitAsConstantBufferView(8);
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dRootParameters[7 + i].InitAsDescriptorTable(1, &pd3dDescriptorRanges[3 + i], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	d3dSamplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
	);



	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void BillboardShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, false);
	m_EffectCB = make_unique<UploadBuffer<CB_EFFECT_INFO>>(pd3dDevice, m_nObjects, false);
}

D3D12_BLEND_DESC BillboardShader::CreateBlendState(int index)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return d3dBlendDesc;
}

D3D12_INPUT_LAYOUT_DESC BillboardShader::CreateInputLayout(int index)
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	inputLayout = { m_pInputElementDesc.data(), (UINT)m_pInputElementDesc.size() };

	return inputLayout;
}

void BillboardShader::Animate(float fTimeElapsed)
{
	Camera* pCamera = m_pCamera;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->isLive()) {
			m_ppObjects[i]->SetLookAt(m_pCamera->GetPosition());
			m_ppObjects[i]->Animate(fTimeElapsed);
		}
	}

}

void BillboardShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
	
	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera);
}

void BillboardShader::SetRotateLockXZ(bool lock)
{
	for (auto& p : m_ppObjects) {
		p->SetRotateXZLock(lock);
	}
}

void BillboardShader::setAnimSpeed(float speed)
{
	for (auto& p : m_ppObjects) {
		reinterpret_cast<PaticleObject*>(p)->m_fAnimationSpeed = speed;
	}
}

void BillboardShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_EFFECT_INFO cEffectBuffer;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);

		cEffectBuffer.m_nMaxXcount = (UINT)reinterpret_cast<EffectInstanceObject*>(m_ppObjects[i])->m_fMaxXCount;
		cEffectBuffer.m_nNowXcount = (UINT)reinterpret_cast<EffectInstanceObject*>(m_ppObjects[i])->m_fNowXCount;

		cEffectBuffer.m_nMaxYcount = (UINT)reinterpret_cast<EffectInstanceObject*>(m_ppObjects[i])->m_fMaxYCount;
		cEffectBuffer.m_nNowYcount = (UINT)reinterpret_cast<EffectInstanceObject*>(m_ppObjects[i])->m_fNowYCount;

		m_EffectCB->CopyData(i, cEffectBuffer);
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_FOG, m_FogCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_MATERIAL, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_LIGHTS, m_LightsCB->Resource()->GetGPUVirtualAddress());
}													  

//////////////////////////////////////////////////////////////////


D3D12_DEPTH_STENCIL_DESC TextureToFullScreen::CreateDepthStencilState(int index)
{
	D3D12_DEPTH_STENCIL_DESC desc;
	::ZeroMemory(&desc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	desc.DepthEnable = false;
	desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.StencilEnable = false;
	desc.StencilReadMask = 0x00;
	desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	return desc;
}

D3D12_BLEND_DESC TextureToFullScreen::CreateBlendState(int index)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return d3dBlendDesc;
}

void TextureToFullScreen::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenTexture, 0, 0); // Texture

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[1];
	pd3dRootParameters[0].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	
	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[1];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void TextureToFullScreen::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSDeferredFullScreen", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSFullScreen", "ps_5_0");

	TextureDataForm* mtexture = (TextureDataForm*)pContext;
	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_texture.c_str(), 0);
	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 0, true);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);


	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);
}

void TextureToFullScreen::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	OnPrepareRender(pd3dCommandList);

	m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

/////////////////////////

DeferredFullScreen::DeferredFullScreen()
{
}

DeferredFullScreen::~DeferredFullScreen()
{
}

void DeferredFullScreen::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[NUM_GBUFFERS + NUM_DIRECTION_LIGHTS];

	for (int i = 0; i < NUM_GBUFFERS; ++i) {
		pd3dDescriptorRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenTexture + i, 0, 0); // Texture
	}

	for(i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dDescriptorRanges[NUM_GBUFFERS + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVShadowMap + i, 0, 0);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[NUM_GBUFFERS + NUM_DIRECTION_LIGHTS + 1];

	pd3dRootParameters[0].InitAsConstantBufferView(10);
	for (int i = 0; i < NUM_GBUFFERS; ++i) {
		pd3dRootParameters[i + 1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);
	}
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dRootParameters[NUM_GBUFFERS + i + 1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[NUM_GBUFFERS + i], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	d3dSamplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
	);



	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void DeferredFullScreen::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	CTexture* pTexture = (CTexture *)pContext;
	m_pTexture = make_unique<CTexture>(*pTexture);

	for(int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		m_pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSDeferredFullScreen", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSDeferredFullScreen", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get() , 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);
}


D3D12_DEPTH_STENCIL_DESC DeferredFullScreen::CreateDepthStencilState(int index)
{
	D3D12_DEPTH_STENCIL_DESC desc;
	::ZeroMemory(&desc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	desc.DepthEnable = false;
	desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.StencilEnable = false;
	desc.StencilReadMask = 0x00;
	desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	return desc;

}

D3D12_BLEND_DESC DeferredFullScreen::CreateBlendState(int index)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return d3dBlendDesc;
}


void DeferredFullScreen::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	OnPrepareRender(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
	
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_BulrCB->Resource()->GetGPUVirtualAddress());

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void DeferredFullScreen::Animate(float fTimeElapsed)
{
	if (*m_pNowScene == 0) {
		m_IsDeath = 0.0f;
		m_Time = 0;
		m_Scale.x = 0;
		m_Scale.y = 0;
		return;
	}
	if (m_pPlayer != nullptr && m_pPlayer->GetStatus()->m_health > 0.0f) return;

	m_IsDeath = 1.0f;
	m_Time += (fTimeElapsed * BLUR_SPEED);
	m_Scale.x = min(static_cast<UINT>(m_Time), MAX_SCALE);
	m_Scale.y = min(static_cast<UINT>(m_Time), MAX_SCALE);
}

void DeferredFullScreen::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_SCENEBLUR_INFO bluerInfo;

	bluerInfo.m_BlurScale = m_Scale;
	bluerInfo.m_Time = m_Time;
	bluerInfo.m_Enable = m_IsDeath;

	m_BulrCB->CopyData(0, bluerInfo);
}

void DeferredFullScreen::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_BulrCB = make_unique<UploadBuffer<CB_SCENEBLUR_INFO>>(pd3dDevice, m_nObjects, true);
}

//////////////////////////////////////////

ShadowDebugShader::ShadowDebugShader()
{
}

ShadowDebugShader::~ShadowDebugShader()
{
}

void ShadowDebugShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;

	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[NUM_DIRECTION_LIGHTS];
	for(i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dDescriptorRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVShadowMap + i, 0, 0);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[NUM_DIRECTION_LIGHTS];
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dRootParameters[i].InitAsDescriptorTable(1, &pd3dDescriptorRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc[2];
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));

	d3dSamplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		1,
		D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
		0.0f,
		D3D12_FLOAT32_MAX,
		D3D12_SHADER_VISIBILITY_PIXEL
	);

	d3dSamplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(
		1, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK
	);

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 2;
	d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void ShadowDebugShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	CTexture* pTexture = (CTexture *)pContext;
	m_pTexture = make_unique<CTexture>(*pTexture);

	m_nPSO = 1;
	m_nObjects = 1;
	CreatePipelineParts();
	
	m_VSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSDeferredFullScreen", "vs_5_0");
	m_PSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PS", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 0, true);
	
	for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i) {
		m_Resource[i] = m_pTexture->GetTexture(i);
		m_UploadBuffer[i] = m_pTexture->GetUploadBuffer(i);
	}
	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);
}

void ShadowDebugShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
}

void ShadowDebugShader::RefreshShdowMap(ID3D12GraphicsCommandList * pd3dCommandList)
{
	OnPrepareRender(pd3dCommandList);
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

ShadowDebugShader * ShadowDebugShader::Instance()
{
	static ShadowDebugShader instance;

	return &instance;
}

////////////////////////////////////////////////////

FadeEffectShader::FadeEffectShader()
{
}

FadeEffectShader::~FadeEffectShader()
{
}

D3D12_BLEND_DESC FadeEffectShader::CreateBlendState(int index)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = true;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC FadeEffectShader::CreateDepthStencilState(int index)
{
	D3D12_DEPTH_STENCIL_DESC desc;
	::ZeroMemory(&desc, sizeof(D3D12_DEPTH_STENCIL_DESC));

	desc.DepthEnable = false;
	desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.StencilEnable = false;
	desc.StencilReadMask = 0x00;
	desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;
	desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_LESS;

	return desc;
}

void FadeEffectShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].InitAsConstantBufferView(7, 0);

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ComPtr<ID3DBlob> pd3dSignatureBlob = NULL;
	ComPtr<ID3DBlob> pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());

	if (pd3dErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void FadeEffectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSDeferredFullScreen", "vs_5_0");
	m_PSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSFadeEffect", "ps_5_0");

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);
}

void FadeEffectShader::Animate(float fTimeElapsed)
{
	if (!m_bFadeOn) return;
	
	m_Color.w +=( m_bFadeType ? (fTimeElapsed / m_fExistTime) : (-fTimeElapsed / m_fExistTime));

	// 페이드 인, 아웃이 끝난 경우
	if ((m_bFadeType && m_Color.w > 1.0f) || (!m_bFadeType && m_Color.w < 0.0f)) {
		m_Color.w = m_bFadeType ? 1.0f : 0.0f;
		m_bFadeOn = m_bAutoChange;
		m_bFadeType = !m_bFadeType;
		m_bAutoChange = false;
	}
	
}

void FadeEffectShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_FadeCB = make_unique<UploadBuffer<XMFLOAT4>>(pd3dDevice, 1, true);
}

void FadeEffectShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_FadeCB->CopyData(0, m_Color);
}

void FadeEffectShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	if (!m_bFadeOn) return;

	OnPrepareRender(pd3dCommandList, 0);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void FadeEffectShader::SetFadeIn(const bool bfadeType, const float fExistTime, const bool autoChange, const XMFLOAT3 & xmf3Color)
{
	m_Color = XMFLOAT4(xmf3Color.x , xmf3Color.y, xmf3Color.z, (bfadeType ? 0.0f : 1.0f));
	m_bFadeOn = true;
	m_bFadeType = bfadeType;
	m_bAutoChange = autoChange;
	m_fExistTime = fExistTime;
}

void FadeEffectShader::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	if (m_RootSignature[index])
		pd3dCommandList->SetGraphicsRootSignature(m_RootSignature[index].Get());

	if (m_pPSO[index])
		pd3dCommandList->SetPipelineState(m_pPSO[index].Get());

	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_FadeCB->Resource()->GetGPUVirtualAddress());

	UpdateShaderVariables(pd3dCommandList);
}

///////////////////////////////////////////


void DrawGBuffers::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	CTexture* pTexture = (CTexture *)pContext;
	m_pTexture = make_unique<CTexture>(*pTexture);

	//for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
	//	m_pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\RTDraw.hlsl", nullptr, "VSRTTextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\RTDraw.hlsl", nullptr, "PSRTTextured", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);
}

void DrawGBuffers::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	OnPrepareRender(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}
