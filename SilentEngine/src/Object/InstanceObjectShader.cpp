#include "stdafx.h"
#include "InstanceObjectShader.h"
#include "Camera.h"
#include "InstanceObject.h"

D3D12_INPUT_LAYOUT_DESC InstanceObjectShader::CreateInputLayout(int index)
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

void InstanceObjectShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{ }


void InstanceObjectShader::CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource* pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	for (int i = 0; i < 1; i++)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dConstantBuffers->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, RESOURCE_BUFFER);

		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_nObjects;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = sizeof(CB_GAMEOBJECT_INFO);

		pd3dDevice->CreateShaderResourceView(pd3dConstantBuffers, &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
	}
}

void InstanceObjectShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	// Instance 데이터는 상수버퍼가 아닌 셰이더 리소스 버퍼에 저장되기 떄문에 마지막 인자를 false로 설정
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, false);
}

void InstanceObjectShader::CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;
	int nTextures = pTexture->GetTextureCount();
	
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

void InstanceObjectShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);
	}
}

void InstanceObjectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{ }

float num = 0.0f;

void InstanceObjectShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
	XMFLOAT3 ttt = m_ppObjects[0]->GetPosition();
	ttt.y -= 0.1f;
	m_ppObjects[0]->SetPosition(ttt);
	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, pCamera);
}

D3D12_INPUT_LAYOUT_DESC InstanceIlluminatedObjectShader::CreateInputLayout(int index)
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	inputLayout = { m_pInputElementDesc.data(), (UINT)m_pInputElementDesc.size() };

	return inputLayout;
}

void InstanceIlluminatedObjectShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{ }

void InstanceIlluminatedObjectShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{ }

void InstanceIlluminatedObjectShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{ }
