#include "stdafx.h"
#include "Camera.h"
#include "Shaders.h"
#include "LightObjects.h"
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
		psoDesc.RTVFormats[i] = m_Format;
	psoDesc.SampleDesc.Count		= 1;
	psoDesc.DSVFormat				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	ThrowIfFailed(pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO[index].GetAddressOf())));
}

void Shaders::BuildComputePSO(ID3D12Device * pd3dDevice, int index)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsoDesc;
	::ZeroMemory(&cpsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

	cpsoDesc.pRootSignature = m_ComputeRootSignature[0].Get();
	cpsoDesc.CS				= CreateComputeShader(index);
	cpsoDesc.Flags			= D3D12_PIPELINE_STATE_FLAG_NONE;

	ThrowIfFailed(pd3dDevice->CreateComputePipelineState(&cpsoDesc, IID_PPV_ARGS(m_pComputePSO[index].GetAddressOf())));
}

void Shaders::CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement)
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
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = {};
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
	case RESOURCE_BUFFER_FLOAT32:
		d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d3dShaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 1;
		break;
	case RESOURCE_TEXTURE2D_HDR:
		d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d3dShaderResourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		break;
	}
	return(d3dShaderResourceViewDesc);
}

void Shaders::CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement, bool bIsGraphics)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle;
	if (bIsGraphics) {
		d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
		d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;
	}

	else {
		d3dSrvCPUDescriptorHandle = m_d3dComputeSrvCPUDescriptorStartHandle;
		d3dSrvGPUDescriptorHandle = m_d3dComputeSrvGPUDescriptorStartHandle;
	}

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

void Shaders::CreateComputeDescriptorHeaps(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews, int nUnorderedAccessViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews + nUnorderedAccessViews; //CBVs + SRVs  + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_ComputeCbvSrvDescriptorHeap);

	ThrowIfFailed(pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		IID_PPV_ARGS(&m_ComputeCbvSrvDescriptorHeap)));

	m_d3dComputeCbvCPUDescriptorStartHandle = m_ComputeCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dComputeCbvGPUDescriptorStartHandle = m_ComputeCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dComputeSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dComputeSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

void Shaders::CreateComputeShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement, XMUINT2* elements)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle;

	d3dSrvCPUDescriptorHandle = m_d3dComputeCbvCPUDescriptorStartHandle;
	d3dSrvGPUDescriptorHandle = m_d3dComputeCbvGPUDescriptorStartHandle;
	
	int nTextures = pTexture->GetTextureCount();
	int nBuffers = m_pComputeUAVBuffers.size();

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

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	for (int i = 0; i < nBuffers; ++i)
	{
		if (elements[i].x == UAFloatBuffer) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
			uavDesc.Buffer.NumElements = elements[i].y;
			
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = elements[i].y;
			srvDesc.Texture2D.MipLevels = 0;
		}
		else if (elements[i].x == UATexBuffer) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Texture2D.MipSlice = 0;
			
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = 0;
			srvDesc.Texture2D.MipLevels = 1;
		}

		pd3dDevice->CreateUnorderedAccessView(m_pComputeUAVBuffers[i].Get(), nullptr, &uavDesc, d3dSrvCPUDescriptorHandle);

		m_pComputeSRVUAVCPUHandles[i * 2] = d3dSrvCPUDescriptorHandle;
		m_pComputeSRVUAVGPUHandles[i * 2] = d3dSrvGPUDescriptorHandle;

		d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

		pd3dDevice->CreateShaderResourceView(m_pComputeOutputBuffers[i].Get(), &srvDesc, d3dSrvCPUDescriptorHandle);
	
		m_pComputeSRVUAVCPUHandles[i * 2 + 1] = d3dSrvCPUDescriptorHandle;
		m_pComputeSRVUAVGPUHandles[i * 2 + 1] = d3dSrvGPUDescriptorHandle;

		d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	
}

void Shaders::CreateCbvAndSrvDescriptorHeaps(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews,  bool bIsGraphics)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	
	if (bIsGraphics) {
		HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_CbvSrvDescriptorHeap);

		ThrowIfFailed(pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
			IID_PPV_ARGS(&m_CbvSrvDescriptorHeap)));

		m_d3dCbvCPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_d3dCbvGPUDescriptorStartHandle = m_CbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
		m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	}
	else {
		HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_ComputeCbvSrvDescriptorHeap);

		ThrowIfFailed(pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
			IID_PPV_ARGS(&m_ComputeCbvSrvDescriptorHeap)));

		m_d3dComputeCbvCPUDescriptorStartHandle = m_ComputeCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_d3dComputeCbvGPUDescriptorStartHandle = m_ComputeCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		m_d3dComputeSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
		m_d3dComputeSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	}
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

D3D12_SHADER_BYTECODE Shaders::CreateComputeShader(int index)
{
	D3D12_SHADER_BYTECODE byteCode;
	::ZeroMemory(&byteCode, sizeof(D3D12_SHADER_BYTECODE));

	if (m_CSByteCode[index] != nullptr) {
		byteCode.pShaderBytecode = m_CSByteCode[index]->GetBufferPointer();
		byteCode.BytecodeLength = m_CSByteCode[index]->GetBufferSize();
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

void Shaders::DispatchComputePipeline(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	pd3dCommandList->SetComputeRootSignature(m_ComputeRootSignature[0].Get());
	
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
	if (m_nPSO > 0) {
		m_pPSO = new ComPtr<ID3D12PipelineState>[m_nPSO];
		m_RootSignature = new ComPtr<ID3D12RootSignature>[m_nPSO];

		m_VSByteCode = new ComPtr<ID3DBlob>[m_nPSO];
		m_PSByteCode = new ComPtr<ID3DBlob>[m_nPSO];
	}

	if (m_nComputePSO > 0) {
		m_ComputeRootSignature = new ComPtr<ID3D12RootSignature>();
		m_pComputePSO = new ComPtr<ID3D12PipelineState>[m_nComputePSO];

		m_pComputeSRVUAVCPUHandles = vector<D3D12_CPU_DESCRIPTOR_HANDLE>(m_nComputeBuffers * 2);
		m_pComputeSRVUAVGPUHandles = vector<D3D12_GPU_DESCRIPTOR_HANDLE>(m_nComputeBuffers * 2);
		
		m_pComputeUAVBuffers = vector<ComPtr<ID3D12Resource>>(m_nComputeBuffers);
		m_pComputeOutputBuffers = vector<ComPtr<ID3D12Resource>>(m_nComputeBuffers);

		m_CSByteCode = new ComPtr<ID3DBlob>[m_nComputePSO];
	}
}

void Shaders::SetMultiUploadBuffer(void** data)
{
	m_LightsCB	= reinterpret_cast<UploadBuffer<LIGHTS>*>(data[0]);
	m_MatCB		= reinterpret_cast<UploadBuffer<MATERIALS>*>(data[1]);
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
{ }

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

void NormalMapShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) { }

void NormalMapShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList) { }

void NormalMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList) { }

void NormalMapShader::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList) { }

void NormalMapShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice) { }

/////////////////////////////////////


void BillboardShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;
	int i = 0;
	
	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2 + NUM_MAX_UITEXTURE];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceData, 0, 0);
	for(int i = 0; i < NUM_MAX_UITEXTURE; ++i) 
		pd3dDescriptorRanges[1 + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVUITextureMap + i, 0, 0);
	pd3dDescriptorRanges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceEffectData, 0, 0);
	
	CD3DX12_ROOT_PARAMETER pd3dRootParameters[3 + NUM_MAX_UITEXTURE];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[5], D3D12_SHADER_VISIBILITY_ALL);
	for (int i = 0; i < NUM_MAX_UITEXTURE; ++i)
		pd3dRootParameters[3 + i].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1 + i]);

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

void BillboardShader::DisableAll()
{
	for (auto& p : m_ppObjects)
		p->SetLive(false);
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

		cEffectBuffer.m_nType = reinterpret_cast<EffectInstanceObject*>(m_ppObjects[i])->m_nType;
		m_EffectCB->CopyData(i, cEffectBuffer);
	}
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

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSFadeEffect", "vs_5_0");
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
	Shaders::BuildPSO(pd3dDevice, nRenderTargets);
}

void TextureToFullScreen::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{

	OnPrepareRender(pd3dCommandList);

	m_pMaterial->UpdateShaderVariables(pd3dCommandList);

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

void DeferredFullScreen::BuildPSO(ID3D12Device * pd3dDevice, UINT nRenderTargets, int index)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	::ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = CreateInputLayout(index);
	psoDesc.pRootSignature = m_RootSignature[index].Get();
	psoDesc.VS = CreateVertexShader(index);
	psoDesc.PS = CreatePixelShader(index);
	psoDesc.RasterizerState = CreateRasterizerState(index);
	psoDesc.BlendState = CreateBlendState(index);
	psoDesc.DepthStencilState = CreateDepthStencilState(index);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = nRenderTargets;
	psoDesc.RTVFormats[0] = m_Format;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ThrowIfFailed(pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pPSO[index].GetAddressOf())));
}

void DeferredFullScreen::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	const int numofCB = 4;

	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[NUM_GBUFFERS + NUM_DIRECTION_LIGHTS];

	for (int i = 0; i < NUM_GBUFFERS; ++i) {
		pd3dDescriptorRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenTexture + i, 0, 0); // Texture
	}

	for(i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dDescriptorRanges[NUM_GBUFFERS + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVShadowMap + i, 0, 0);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[NUM_GBUFFERS + NUM_DIRECTION_LIGHTS + numofCB];

	pd3dRootParameters[0].InitAsConstantBufferView(10);
	pd3dRootParameters[1].InitAsConstantBufferView(CBVMaterial);
	pd3dRootParameters[2].InitAsConstantBufferView(CBVLights);
	pd3dRootParameters[3].InitAsConstantBufferView(CBVCameraInfo);

	for (int i = 0; i < NUM_GBUFFERS; ++i) {
		pd3dRootParameters[i + numofCB].InitAsDescriptorTable(1, &pd3dDescriptorRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);
	}
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dRootParameters[NUM_GBUFFERS + i + numofCB].InitAsDescriptorTable(1, &pd3dDescriptorRanges[NUM_GBUFFERS + i], D3D12_SHADER_VISIBILITY_PIXEL);


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
	m_Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	for(int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		m_pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\DefferredShader.hlsl", nullptr, "VS_DEFFERED_SHADER", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\DefferredShader.hlsl", nullptr, "PS_DEFFERED_SHADER", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get() , 4, true);

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
	pCamera->UpdateShaderVariables(pd3dCommandList);
	
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
	
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_BulrCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, MATERIAL_MANAGER->MaterialUploadBuffer()->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, LIGHT_MANAGER->LightUploadBuffer()->Resource()->GetGPUVirtualAddress());

	if( pCamera != nullptr)
		pd3dCommandList->SetGraphicsRootConstantBufferView(3, pCamera->GetUploadBuffer()->Resource()->GetGPUVirtualAddress());
	
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

	LIGHT_MANAGER->UpdateShaderVariables();
	MATERIAL_MANAGER->UpdateShaderVariables();

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
	Shaders::BuildPSO(pd3dDevice, nRenderTargets);
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

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexture2D, 0, 0);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[2];
	pd3dRootParameters[0].InitAsConstantBufferView(7, 0);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);

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

void FadeEffectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nPSO = 1;
	CreatePipelineParts();

	TextureDataForm* mtexture = (pContext == NULL) ? nullptr : (TextureDataForm*)pContext;
	
	if (mtexture != nullptr) {
		m_pTexture = make_unique<CTexture>(1, RESOURCE_TEXTURE2D, 0);
		m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_texture.c_str(), 0);
	}

	m_VSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSFadeEffect", "vs_5_0");
	m_PSByteCode[PSO_OBJECT] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSFadeEffect", "ps_5_0");

	if (mtexture != nullptr) {
		CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 1, true);
	}
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateGraphicsRootSignature(pd3dDevice);
	Shaders::BuildPSO(pd3dDevice, nRenderTargets);
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

	
	if (m_pTexture) {
		Shaders::OnPrepareRender(pd3dCommandList, 0);
		m_pTexture->UpdateShaderVariables(pd3dCommandList);
	}
	else
		OnPrepareRender(pd3dCommandList, 0);

	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_FadeCB->Resource()->GetGPUVirtualAddress());

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

	UpdateShaderVariables(pd3dCommandList);
}

///////////////////////////////////////////


void DrawGBuffers::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
}

void DrawGBuffers::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	CTexture* pTexture = (CTexture *)pContext;
	m_pTexture = make_unique<CTexture>(*pTexture);

	for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		m_pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\RTDraw.hlsl", nullptr, "VSRTTextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\RTDraw.hlsl", nullptr, "PSRTTextured", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 4, true);
	
	CreateGraphicsRootSignature(pd3dDevice);
	Shaders::BuildPSO(pd3dDevice, nRenderTargets);
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

///////////////////////////////////////////////////


void HDRShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_HDRDownScaleCB = make_unique<UploadBuffer<CB_HDR_DOWNSCALE_INFO>>(pd3dDevice, 1, true);

	m_HDRDownScaleData.m_Res = XMUINT2(FRAME_BUFFER_WIDTH / 4, FRAME_BUFFER_HEIGHT / 4);
	m_HDRDownScaleData.m_Domain = m_HDRDownScaleData.m_Res.x * m_HDRDownScaleData.m_Res.y;
	m_HDRDownScaleData.m_GroupSize = (UINT)ceil((float)(FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT / 16) / 1024.0f);
	m_HDRDownScaleData.m_BloomThreshold = 3.5f;

	m_nComputeThreadCount[DownScaleFirstPass]	= XMUINT3(m_HDRDownScaleData.m_GroupSize, 1, 1);
	m_nComputeThreadCount[DownScaleSecondPass]	= XMUINT3(1, 1, 1);
	m_nComputeThreadCount[BloomAvgLum]			= XMUINT3{ m_HDRDownScaleData.m_GroupSize, 1, 1 };
	m_nComputeThreadCount[BloomBlurHorizon]		= XMUINT3{ (UINT)ceil(((float)m_HDRDownScaleData.m_Res.x) / (128.0f - 12.0f)), m_HDRDownScaleData.m_Res.y, 1 };
	m_nComputeThreadCount[BloomBlurVertical]	= XMUINT3{ m_HDRDownScaleData.m_Res.x, (UINT)ceil(((float)m_HDRDownScaleData.m_Res.y) / (128.0f - 12.0f)), 1 };

	m_HDRToneMappCB = make_unique<UploadBuffer<CB_HDR_TONEMAPPING_INFO>>(pd3dDevice, 1, true);

	m_HDRToneMappData.m_LumWhiteSqr = 1.5f;
	m_HDRToneMappData.m_MiddleGrey = 0.35f;
	m_HDRToneMappData.m_EnableHDR = 1.0f;
	m_HDRToneMappData.m_EnableBloom = 1.0f;
}

void HDRShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[NUM_HDRBUFFER + 2];

	for (i = 0; i < NUM_HDRBUFFER; ++i)
		pd3dDescriptorRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenHDR, 0, 0);
	pd3dDescriptorRanges[NUM_HDRBUFFER].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVAverageValues);
	pd3dDescriptorRanges[NUM_HDRBUFFER + 1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVBloomInput);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[NUM_HDRBUFFER + 4];

	pd3dRootParameters[0].InitAsConstantBufferView(CBVHDRDownScale);
	pd3dRootParameters[1].InitAsConstantBufferView(CBVHDRToneMapp);
	for (i = 0; i < NUM_HDRBUFFER; ++i)
		pd3dRootParameters[i + 2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[NUM_HDRBUFFER + 2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[NUM_HDRBUFFER + 3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_ALL);
	
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

void HDRShader::CreateComputeRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenHDR, 0, 0);
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVAverageValues1D);
	pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVAverageValues);
	pd3dDescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, UAVAverageLum);
	pd3dDescriptorRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, UAVHDRDownScale);
	pd3dDescriptorRanges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVHDRDownScale);
	pd3dDescriptorRanges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, UAVBloom);
	pd3dDescriptorRanges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVBloomInput);
	pd3dDescriptorRanges[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, UAVBloomOutput);

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[10];
	pd3dRootParameters[CBDownScale].InitAsConstantBufferView(CBVHDRDownScale);
	pd3dRootParameters[SRHDR].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[SRAverageValues1DOutput].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[SRAverageValuesOutput].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[UAAverageLumInput].InitAsDescriptorTable(1, &pd3dDescriptorRanges[3], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[UAHDRDownScale].InitAsDescriptorTable(1, &pd3dDescriptorRanges[4], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[SRHDRDownScale].InitAsDescriptorTable(1, &pd3dDescriptorRanges[5], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[UABloom].InitAsDescriptorTable(1, &pd3dDescriptorRanges[6], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[SRBloomInput].InitAsDescriptorTable(1, &pd3dDescriptorRanges[7], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[UABloomOutput].InitAsDescriptorTable(1, &pd3dDescriptorRanges[8], D3D12_SHADER_VISIBILITY_ALL);

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
		IID_PPV_ARGS(m_ComputeRootSignature[PSO_OBJECT].GetAddressOf()))
	);
}

void HDRShader::CreateUAVResourceView(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT descriptorIndex, UINT nTextureIndex, UINT numElements)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = numElements;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor = m_d3dComputeSrvCPUDescriptorStartHandle;
	descriptor.ptr += ::gnCbvSrvDescriptorIncrementSize * descriptorIndex;
	pd3dDevice->CreateUnorderedAccessView(pTexture->GetTexture(nTextureIndex).Get(), nullptr, &uavDesc, descriptor);
}

void HDRShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{

	CTexture* pTexture = (CTexture *)pContext;

	m_pTexture = make_unique<CTexture>(*pTexture);
	m_pComputeTexture = make_unique<CTexture>(*pTexture);
	
	m_nObjects = 1;
	m_nPSO = 1;
	m_nComputePSO = 5;
	m_nComputeBuffers = 6;

	CreatePipelineParts();								// ok
	CreateComputeBuffer(pd3dDevice, pd3dCommandList);	// ok

	m_pTexture->AddTexture(m_pComputeOutputBuffers[DownScaleSecondPass].Get(), nullptr, RESOURCE_BUFFER_FLOAT32);
	m_pTexture->AddTexture(m_pComputeOutputBuffers[HandleHorizonBloomBuffer].Get(), nullptr, RESOURCE_TEXTURE2D_HDR);

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\HDRShader.hlsl", nullptr, "VSHDR", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\HDRShader.hlsl", nullptr, "PSHDR", "ps_5_0");
	m_CSByteCode[DownScaleFirstPass] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\ComputeShaders.hlsl", nullptr, "DownScaleFirstPass", "cs_5_0");
	m_CSByteCode[DownScaleSecondPass] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\ComputeShaders.hlsl", nullptr, "DownScaleSecondPass", "cs_5_0");
	m_CSByteCode[BloomAvgLum] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\ComputeShaders.hlsl", nullptr, "BloomPass", "cs_5_0");
	m_CSByteCode[BloomBlurVertical] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\ComputeShaders.hlsl", nullptr, "VerticalBloomFilter", "cs_5_0");
	m_CSByteCode[BloomBlurHorizon] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\ComputeShaders.hlsl", nullptr, "HorizonBloomFilter", "cs_5_0");

	
	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateComputeDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pComputeTexture->GetTextureCount(), m_nComputeBuffers * 2);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 2, true);
	CreateComputeShaderResourceViews(pd3dDevice, pd3dCommandList, m_pComputeTexture.get(), 1, true, m_nBuffTypeElements);

	CreateGraphicsRootSignature(pd3dDevice);
	CreateComputeRootSignature(pd3dDevice);

	Shaders::BuildPSO(pd3dDevice, nRenderTargets);

	for(int i = DownScaleFirstPass; i <= BloomBlurHorizon; ++i ) 
		BuildComputePSO(pd3dDevice, i);
	
}

void HDRShader::CreateComputeBuffer(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_nBuffTypeElements = new XMUINT2[m_nComputeBuffers];
	m_nComputeThreadCount = new XMUINT3[m_nComputePSO];

	XMUINT2 byteSize = XMUINT2{ 4 * (UINT)ceil((float)(FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT / 16) / 1024.0f) , 0 };
	D3DUtil::CreateUnorderedAccessResource(pd3dDevice, pd3dCommandList, byteSize, m_pComputeUAVBuffers[FirstPassAverageLumBuffer], m_pComputeOutputBuffers[FirstPassAverageLumBuffer], UAFloatBuffer);
	m_nBuffTypeElements[FirstPassAverageLumBuffer] = { UAFloatBuffer , byteSize.x / 4 };
	
	byteSize.x = 4;
	D3DUtil::CreateUnorderedAccessResource(pd3dDevice, pd3dCommandList, byteSize, m_pComputeUAVBuffers[SecondPassAverageLumBuffer], m_pComputeOutputBuffers[SecondPassAverageLumBuffer], UAFloatBuffer);
	m_nBuffTypeElements[SecondPassAverageLumBuffer] = { UAFloatBuffer , byteSize.x / 4 };

	byteSize = { FRAME_BUFFER_WIDTH / 4, FRAME_BUFFER_HEIGHT / 4};

	for (int i = HDRDownScaleBuffer; i <= HandleHorizonBloomBuffer; ++i) {
		D3DUtil::CreateUnorderedAccessResource(pd3dDevice, pd3dCommandList, byteSize, m_pComputeUAVBuffers[i], m_pComputeOutputBuffers[i], UATexBuffer);
		m_nBuffTypeElements[i] = { UATexBuffer , 0 };
	}

}

void HDRShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_HDRToneMappData.m_EnableHDR = HDR_ON ? 1.0f : 0.0f;
	m_HDRToneMappData.m_EnableBloom = BLOOM_ON ? 1.0f : 0.0f;

	m_HDRDownScaleCB->CopyData(0, m_HDRDownScaleData);
	m_HDRToneMappCB->CopyData(0, m_HDRToneMappData);

	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_HDRDownScaleCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, m_HDRToneMappCB->Resource()->GetGPUVirtualAddress());
}

void HDRShader::DispatchComputePipeline(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	pd3dCommandList->SetPipelineState(m_pComputePSO[index].Get());
	pd3dCommandList->SetComputeRootSignature(m_ComputeRootSignature[0].Get());
	pd3dCommandList->SetDescriptorHeaps(1, m_ComputeCbvSrvDescriptorHeap.GetAddressOf());
	
	m_pComputeTexture->UpdateComputeShaderVariables(pd3dCommandList);
	//CBDownScale, SRHDR, SRAverageValues1DOutput, SRAverageValuesOutput, UAAverageLumInput
	switch (index)
	{
	case DownScaleFirstPass:
		pd3dCommandList->SetComputeRootConstantBufferView(0, m_HDRDownScaleCB->Resource()->GetGPUVirtualAddress());

		pd3dCommandList->SetComputeRootDescriptorTable(UAAverageLumInput, m_pComputeSRVUAVGPUHandles[HandleFirstPassAverageLumUAV]);
		pd3dCommandList->SetComputeRootDescriptorTable(UAHDRDownScale, m_pComputeSRVUAVGPUHandles[HandleHDRDownScaleUAV]);
		break;

	case DownScaleSecondPass:
		pd3dCommandList->SetComputeRootConstantBufferView(0, m_HDRDownScaleCB->Resource()->GetGPUVirtualAddress());

		pd3dCommandList->SetComputeRootDescriptorTable(SRAverageValues1DOutput, m_pComputeSRVUAVGPUHandles[HandleFirstPassAverageLumSRV]);
		pd3dCommandList->SetComputeRootDescriptorTable(UAAverageLumInput, m_pComputeSRVUAVGPUHandles[HandleSecondPassAverageLumUAV]);
		break;

	case BloomAvgLum:
		// gHDRDownScaleTexture gAverageLum : SRV 
		pd3dCommandList->SetComputeRootDescriptorTable(SRAverageValuesOutput, m_pComputeSRVUAVGPUHandles[HandleSecondPassAverageLumSRV]);
		pd3dCommandList->SetComputeRootDescriptorTable(SRHDRDownScale, m_pComputeSRVUAVGPUHandles[HandleHDRDownScaleSRV]);
		
		// gBloom : UAV
		pd3dCommandList->SetComputeRootDescriptorTable(UABloom, m_pComputeSRVUAVGPUHandles[HandleAverageBloomUAV]);
		break;

	case BloomBlurVertical:
		// gBloomInput : SRV
		pd3dCommandList->SetComputeRootDescriptorTable(SRBloomInput, m_pComputeSRVUAVGPUHandles[HandleAverageBloomSRV]);

		// gBloomOutput : UAV
		pd3dCommandList->SetComputeRootDescriptorTable(UABloomOutput, m_pComputeSRVUAVGPUHandles[HandleVerticalBloomUAV]);
		break;

	case BloomBlurHorizon:
		// gBloomInput : SRV
		pd3dCommandList->SetComputeRootDescriptorTable(SRBloomInput, m_pComputeSRVUAVGPUHandles[HandleVerticalBloomSRV]);

		// gBloomOutput : UAV
		pd3dCommandList->SetComputeRootDescriptorTable(UABloomOutput, m_pComputeSRVUAVGPUHandles[HandleHorizonBloomUAV]);
		break;
	}

	
	pd3dCommandList->Dispatch(m_nComputeThreadCount[index].x, m_nComputeThreadCount[index].y, m_nComputeThreadCount[index].z);
}

void HDRShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	OnPrepareRender(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void HDRShader::Dispatch(ID3D12GraphicsCommandList * pd3dCommandList)
{
	if (!HDR_ON && !BLOOM_ON) return;
	for (int i = DownScaleFirstPass; i <= BloomBlurHorizon; ++i) {
		if (i > DownScaleSecondPass && !BLOOM_ON) return;
		DispatchComputePipeline(pd3dCommandList, i);
		GetUADataInCurrentPipeline(pd3dCommandList, i);
	}
}

void HDRShader::UpGreyScale(float addScale)
{
	m_HDRToneMappData.m_MiddleGrey += addScale;
	std::cout << "MiddGrey : " << m_HDRToneMappData.m_MiddleGrey << std::endl;
}

void HDRShader::UpWhiteScale(float addScale)
{
	m_HDRToneMappData.m_LumWhiteSqr += addScale;
	std::cout << "LumWhiteSqr : " << m_HDRToneMappData.m_LumWhiteSqr << std::endl;
}

void HDRShader::UpBloomScale(float addScale)
{
	m_HDRDownScaleData.m_BloomThreshold += addScale;
	std::cout << "BloomThreshold : " << m_HDRDownScaleData.m_BloomThreshold << std::endl;
}

void HDRShader::GetUAVData(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_pComputeUAVBuffers[index].Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	));

	pd3dCommandList->CopyResource(m_pComputeOutputBuffers[index].Get(), m_pComputeUAVBuffers[index].Get());

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_pComputeUAVBuffers[index].Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_COMMON
	));
}

void HDRShader::GetUADataInCurrentPipeline(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	switch (index)
	{
	case DownScaleFirstPass:
		GetUAVData(pd3dCommandList, FirstPassAverageLumBuffer);
		GetUAVData(pd3dCommandList, HDRDownScaleBuffer);
		break;

	case DownScaleSecondPass:
		GetUAVData(pd3dCommandList, SecondPassAverageLumBuffer);
		break;

	case BloomAvgLum:
		GetUAVData(pd3dCommandList, HandleAverageBloomBuffer);
		break;

	case BloomBlurVertical:
		GetUAVData(pd3dCommandList, HandleVerticalBloomBuffer);
		break;

	case BloomBlurHorizon:
		GetUAVData(pd3dCommandList, HandleHorizonBloomBuffer);
		break;
	}
}

void HDRShader::DebugOutputBuffer(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	UINT size = 0;
	if (index == 0) {
		size = (UINT)ceil((float)(FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT / 16) / 1024.0f);
	}

	if (index != 0) {
		size = 1;
	}

	float* buf = new float[size];
	D3D12_RANGE range = D3D12_RANGE{ 0, m_nComputeThreadCount[index].x };
	ThrowIfFailed(m_pComputeOutputBuffers[index]->Map(0, &range, reinterpret_cast<void**>(&buf)));

	for (int i = 0; i < size / 4; ++i) {
		std::cout << buf[i] << "\t";
	}
	std::cout << std::endl;

	m_pComputeOutputBuffers[index]->Unmap(0, nullptr);
}

/////////////////////////////////////////////////////


void OutlineFogShader::BuildFog()
{
	m_pFog = new CB_FOG_INFO();
	::ZeroMemory(m_pFog, sizeof(CB_FOG_INFO));

	m_pFog->m_xmf4FogColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

	float fFogMode = 1.0f;
	float fStart = 50.0f;
	float fEnd = 400.0f;
	float fDensity = 1.0f;
	m_pFog->m_xmf4Foginfo = XMFLOAT4(fFogMode, fStart, fEnd, fDensity);
}

void OutlineFogShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	BuildFog();

	CTexture* pTexture = (CTexture *)pContext;
	m_pTexture = make_unique<CTexture>(*pTexture);

	m_nObjects = 1;
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\DefferredShader.hlsl", nullptr, "VS_DEFFERED_SHADER", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\DefferredShader.hlsl", nullptr, "PS_FOG_OUTLINE_SHADE", "ps_5_0");

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture.get(), 2, true);

	CreateGraphicsRootSignature(pd3dDevice);
	Shaders::BuildPSO(pd3dDevice, nRenderTargets);
}

void OutlineFogShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_pFog->m_xmf4Foginfo.x = GlobalVal::getInstance()->getFogEnable() ? 1.0f : 0.0f;
	m_FogCB->CopyData(0, *m_pFog);
}

void OutlineFogShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_FogCB = std::make_unique<UploadBuffer<CB_FOG_INFO>>(pd3dDevice, 1, true);
}

void OutlineFogShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	OnPrepareRender(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->SetGraphicsRootConstantBufferView(0, m_FogCB->Resource()->GetGPUVirtualAddress()); //Materials

	if (pCamera != nullptr)
		pd3dCommandList->SetGraphicsRootConstantBufferView(1, pCamera->GetUploadBuffer()->Resource()->GetGPUVirtualAddress());

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void OutlineFogShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	const int numofCB = 2;

	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[NUM_GBUFFERS + 1];

	for (int i = 0; i < NUM_GBUFFERS + 1; ++i) {
		pd3dDescriptorRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVFullScreenTexture + i, 0, 0); // Texture
	}

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[NUM_GBUFFERS + numofCB + 1];

	pd3dRootParameters[0].InitAsConstantBufferView(CBVFog);
	pd3dRootParameters[1].InitAsConstantBufferView(CBVCameraInfo);

	for (int i = 0; i < NUM_GBUFFERS + 1; ++i) {
		pd3dRootParameters[i + numofCB].InitAsDescriptorTable(1, &pd3dDescriptorRanges[i], D3D12_SHADER_VISIBILITY_PIXEL);
	}

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


void MonsterHPShaders::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;
	int i = 0;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVHPBarData, 0, 0);
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceData, 0, 0);
	pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexture2D, 0, 0);
	
	CD3DX12_ROOT_PARAMETER pd3dRootParameters[4];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);

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

void MonsterHPShaders::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_MonsterRatioCB = make_unique<UploadBuffer<CB_MONSTER_INFO>>(pd3dDevice, m_nObjects, false);
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, false);
}

void MonsterHPShaders::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_MONSTER_INFO cMonsterInfo;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		if (i >= m_nMonsters) {
			cMonsterInfo.m_fLive = 0.0f;
			m_MonsterRatioCB->CopyData(i, cMonsterInfo);
			continue;
		}
		XMFLOAT3 monPos = m_ppMonsters[i]->GetPosition();
		m_ppObjects[i]->SetPosition(Vector3::Add(GetHPPos(m_nMonType), monPos));

		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);

		Enemy* enemy = dynamic_cast<Enemy*>(m_ppMonsters[i]);
		Status* stat = enemy->GetStatus();

		cMonsterInfo.m_fHPRatio = static_cast<float>(stat->m_health) / static_cast<float>(stat->m_maxhealth);
		cMonsterInfo.m_fLive = enemy->isLive() ? 1.0f : 0.0f;
		m_MonsterRatioCB->CopyData(i, cMonsterInfo);
	}
}

void MonsterHPShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 10;

	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSHPBar", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSHPBar", "ps_5_0");

	TextureDataForm* mtexture = (TextureDataForm*)pContext;

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_texture.c_str(), 0);

	unsigned int i = 0;

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 3);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_MonsterRatioCB->Resource(), 1, i++, sizeof(XMFLOAT4), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 2, i++, sizeof(CB_GAMEOBJECT_INFO), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 3, 2, false);
	
	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	i = 0;
	CBoardMeshIlluminatedTextured *pBoard = new CBoardMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 40.0f, 4.0f, 0.0f);

	m_ppObjects = vector<GameObject*>(m_nObjects);

	GameObject* pInstnaceObject = new GameObject();
	pInstnaceObject->SetMesh(0, pBoard);
	pInstnaceObject->SetRootParameterIndex(2);
	pInstnaceObject->SetPosition(0, 0, 0);
	pInstnaceObject->SetRotateXZLock(true);
	pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 1));

	m_ppObjects[i++] = pInstnaceObject;

	for (UINT i = 1; i < m_nObjects; ++i) {
		GameObject* pGameObjects = new GameObject();
		pGameObjects->SetMesh(0, pBoard);
		pGameObjects->SetPosition(0, 0, 0);
		pGameObjects->SetRotateXZLock(true);
		pGameObjects->SetRootParameterIndex(2);
		pGameObjects->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize) * 1);
		m_ppObjects[i] = pGameObjects;
	}
}

void MonsterHPShaders::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	pCamera->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->SetGraphicsRootDescriptorTable(1, m_d3dCbvGPUDescriptorStartHandle);


	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera);
}

XMFLOAT3 MonsterHPShaders::GetHPPos(UINT modelIndex)
{
	switch(modelIndex) {
	case MON_GOLEM:		return XMFLOAT3(0.0f, 75.0f, 0.0f);

	case MON_GHOST:		return XMFLOAT3(0.0f, 40.0f, 0.0f);

	case MON_RHINO:		return XMFLOAT3(0.0f, 60.0f, 0.0f);

	case MON_ARM:		return XMFLOAT3(0.0f, 40.0f, 0.0f);
		
	case MON_SKULL:		return XMFLOAT3(0.0f, 60.0f, 0.0f);

	case MON_RICH:		return XMFLOAT3(0.0f, 100.0f, 0.0f);

	case MON_MUTANT:	return XMFLOAT3(0.0f, 75.0f, 0.0f);
	}
	
	return XMFLOAT3();
}
