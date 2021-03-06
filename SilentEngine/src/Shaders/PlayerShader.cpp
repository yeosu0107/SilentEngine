#include "stdafx.h"
#include "PlayerShader.h"
#include "..\Object\Player.h"


PlayerShader::PlayerShader(int index, Camera* camera) :
	DynamicModelShader(index)
{
	m_myCamera = camera;
}

PlayerShader::~PlayerShader()
{
}

void PlayerShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;

	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[5];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, SRVTexture2D, 0, 0); // GameObject
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVMultiTexture, 0, 0); // Texture
	pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVMultiTexture + 1, 0, 0);
	pd3dDescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVMultiTexture + 2, 0, 0); // Player NormalMap
	pd3dDescriptorRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVMultiTexture + 3, 0, 0); // Sword NormalMap
	
	CD3DX12_ROOT_PARAMETER pd3dRootParameters[6];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[3], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[5].InitAsDescriptorTable(1, &pd3dDescriptorRanges[4], D3D12_SHADER_VISIBILITY_PIXEL);

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

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_SHADOWMAP].GetAddressOf()))
	);
}


void PlayerShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nPSO = 2;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\multiTexturemodel.hlsl", nullptr, "VSMultiTexDynamicModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\multiTexturemodel.hlsl", nullptr, "PSMultiTexDynamicModel", "ps_5_0");

	m_VSByteCode[1] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\multiTexturemodel.hlsl", nullptr, "VSMultiTexDynamicModel", "vs_5_0");
	m_PSByteCode[1] = nullptr;

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 4);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));

	CreateGraphicsRootSignature(pd3dDevice);

	BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
	BuildPSO(pd3dDevice, nRenderTargets, PSO_OBJECT);

	ModelLoader* globalModels = GlobalVal::getInstance()->getModelLoader();

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(4, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\Sword.dds", 1);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\c_texture_NRM.dds", 2);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\Sword_NRM.dds", 3);
		
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, true);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	string* name = new string("player");
	Player* tmp = new Player(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	tmp->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
	tmp->SetPhysController((BasePhysX*)pContext, tmp->getCollisionCallback(), &PxExtendedVec3(0, -170, 0));
	
	if (m_myCamera)
		tmp->SetCamera(m_myCamera, (BasePhysX*)pContext);
	tmp->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	tmp->SetRootParameterIndex(1);
	m_ppObjects[0] = tmp;
}

void PlayerShader::Animate(float fTimeElapsed)
{
	ModelShader::Animate(fTimeElapsed);
}

void PlayerShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_DYNAMICOBJECT_INFO cBone;
	for (UINT i = 0; i < m_nObjects; ++i) {
		//cBone.m_xmf4x4World = m_ppObjects[i]->m_xmf4x4World;
		XMStoreFloat4x4(&cBone.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBone.m_nMaterial = 0;

		memcpy(cBone.m_bone, m_ppObjects[i]->GetBoneData(), sizeof(XMFLOAT4X4) * m_ppObjects[i]->GetBoneNum());

		m_BoneCB->CopyData(i, cBone);
	}
}
