#include "stdafx.h"
#include "ModelShader.h"
#include "ModelLoader.h"

ModelShader::ModelShader() : modelIndex(0)
{

}

ModelShader::ModelShader(UINT index) :
	modelIndex(index)
{
}


ModelShader::~ModelShader()
{
}

D3D12_INPUT_LAYOUT_DESC ModelShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,			0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,			12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0,				24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BORNINDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT, 0,		32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	inputLayout = { m_pInputElementDesc.data(), (UINT)m_pInputElementDesc.size() };

	return inputLayout;
}


//void ModelShader::BuildPSO(ID3D12Device * pd3dDevice, UINT nRenderTargets)
//{
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
//	::ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//
//	psoDesc.InputLayout = CreateInputLayout();
//	psoDesc.pRootSignature = m_RootSignature.Get();
//	psoDesc.VS = CreateVertexShader();
//	psoDesc.PS = CreatePixelShader();
//	psoDesc.RasterizerState = CreateRasterizerState();
//	psoDesc.BlendState = CreateBlendState();
//	psoDesc.DepthStencilState = CreateDepthStencilState();
//	psoDesc.SampleMask = UINT_MAX;
//	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	psoDesc.NumRenderTargets = nRenderTargets;
//	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//	psoDesc.SampleDesc.Count = 1;
//	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	ThrowIfFailed(pd3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO)));
//}

void ModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	// 오브젝트 수 만큼 메모리 할당 
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, true);		

	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();

	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	pd3dDevice->CreateConstantBufferView(
		&cbvDesc,
		m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void ModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	for (int i = 0; i < m_nObjects; ++i) {
		cBuffer.m_xmf4x4World = m_ppObjects[i]->m_xmf4x4World;
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);
	}
}

void ModelShader::ReleaseShaderVariables()
{

}


void ModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSModelTextured", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSTextured", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	string matName = globalModels->getMat(modelIndex);
	wstring convert(matName.begin(), matName.end());


	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\pirate.dds", 0);
	
	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateGraphicsRootSignature(pd3dDevice);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, false);
	BuildPSO(pd3dDevice);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	m_ppObjects[0] = new ModelObject(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	//m_ppObjects[0]->SetMesh(0, new MeshGeometryCube(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f));
	m_ppObjects[0]->SetPosition(0, 0, 0);
	m_ppObjects[0]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));

}

void ModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

void ModelShader::Animate(float fTimeElapsed)
{
	for (UINT i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i]->Animate(fTimeElapsed);
	}
}

DynamicModelShader::DynamicModelShader(int index) : ModelShader(index)
{
}

DynamicModelShader::~DynamicModelShader()
{
}


void DynamicModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	//위치값
	ModelShader::CreateShaderVariables(pd3dDevice, pd3dCommandList);
	// 뼈정보
	m_BoneCB = make_unique<UploadBuffer<CB_DYNAMICOBJECT_INFO>>(pd3dDevice, m_nObjects, true);

	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_BoneCB->Resource()->GetGPUVirtualAddress();

	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	pd3dDevice->CreateConstantBufferView(
		&cbvDesc,
		m_CbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void DynamicModelShader::UpdateShaderVariable(ID3D12GraphicsCommandList * pd3dCommandList, XMFLOAT4X4 * pxmf4x4World)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_DYNAMICOBJECT_INFO cBone;
	for (int i = 0; i < m_nObjects; ++i) {
		cBuffer.m_xmf4x4World = m_ppObjects[i]->m_xmf4x4World;
		cBuffer.m_nMaterial = 0;

		memcpy(cBone.m_bone, m_ppObjects[i]->GetBoneData(), sizeof(XMFLOAT4X4) * m_ppObjects[i]->GetBoneNum());
		
		m_ObjectCB->CopyData(i, cBuffer);
		m_BoneCB->CopyData(i, cBone);
	}
}

void DynamicModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModelTextured", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSTextured", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	string matName = globalModels->getMat(modelIndex);
	wstring convert(matName.begin(), matName.end());


	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\pirate.dds", 0);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 2, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));
	CreateGraphicsRootSignature(pd3dDevice);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, false);
	BuildPSO(pd3dDevice);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	m_ppObjects[0] = new ModelObject(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	//m_ppObjects[0]->SetMesh(0, new MeshGeometryCube(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f));
	m_ppObjects[0]->SetPosition(0, 0, 0);
	m_ppObjects[0]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
}
