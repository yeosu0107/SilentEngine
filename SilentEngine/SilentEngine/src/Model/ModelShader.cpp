#include "stdafx.h"
#include "ModelShader.h"
#include "ModelLoader.h"
#include "..\Object\Player.h"

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

void ModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	// 오브젝트 수 만큼 메모리 할당 
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, true);		

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

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_GAMEOBJECT_INFO)));
	CreateGraphicsRootSignature(pd3dDevice);
	
	BuildPSO(pd3dDevice);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	

	ModelObject* object = new ModelObject(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	object->SetPosition(XMFLOAT3(0, 0, 0));
	object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	//object->SetScale(0.1f);
	object->SetPhysMesh(globalPhysX, PhysMesh::Mesh_Tri);
	m_ppObjects[0]=object;
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
	// 뼈정보
	m_BoneCB = make_unique<UploadBuffer<CB_DYNAMICOBJECT_INFO>>(pd3dDevice, m_nObjects, true);
}

void DynamicModelShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 3; //Game Objects
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; //Texture2DArray
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; // Object
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Materials
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 5; //Lights
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //Texture
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
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
		IID_PPV_ARGS(m_RootSignature.GetAddressOf()))
	);
}

void DynamicModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_DYNAMICOBJECT_INFO cBone;
	for (int i = 0; i < m_nObjects; ++i) {
		//cBone.m_xmf4x4World = m_ppObjects[i]->m_xmf4x4World;
		XMStoreFloat4x4(&cBone.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBone.m_nMaterial = 0;

		memcpy(cBone.m_bone, m_ppObjects[i]->GetBoneData(), sizeof(XMFLOAT4X4) * m_ppObjects[i]->GetBoneNum());

		m_BoneCB->CopyData(i, cBone);
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(2, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, m_LightsCB->Resource()->GetGPUVirtualAddress());
}

void DynamicModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModelTextured", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSTextured", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	
	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));
	
	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	

	//ModelObject* tmp = new ModelObject(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	Player* tmp=new Player(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	tmp->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
	tmp->SetPosition(XMFLOAT3(0, 0, 0));
	tmp->SetPhysController(globalPhysX->getCapsuleController());
	tmp->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	m_ppObjects[0] = tmp;
	////m_ppObjects[0]->SetMesh(0, new MeshGeometryCube(pd3dDevice, pd3dCommandList, 10.0f, 10.0f, 10.0f));
	//m_ppObjects[0]->SetPosition(0, 0, 0);
	
}

void DynamicModelShader::Animate(float fTimeElapsed)
{
	ModelShader::Animate(fTimeElapsed);
}

