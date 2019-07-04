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

D3D12_INPUT_LAYOUT_DESC ModelShader::CreateInputLayout(int index)
{
	D3D12_INPUT_LAYOUT_DESC inputLayout;

	m_pInputElementDesc =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0,	36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BORNINDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT,	0,	44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXINDEX",	0, DXGI_FORMAT_R32_UINT,			0,	72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	for (UINT i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBuffer.m_nMaterial = 0;
		m_ObjectCB->CopyData(i, cBuffer);
	}
}

void ModelShader::ReleaseShaderVariables()
{

}


void ModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	ModelLoader* globalModels = GlobalVal::getInstance()->getModelLoader();
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSStaticModel", "ps_5_0");

	m_nObjects = 0;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_GAMEOBJECT_INFO)));
	CreateGraphicsRootSignature(pd3dDevice);
	
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	
}

void ModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	for (UINT j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] && m_ppObjects[j]->isLive()) 
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

void ModelShader::RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera, XMFLOAT3& cameraPos, float offset)
{
	Shaders::OnPrepareRender(pd3dCommandList, PSO_SHADOWMAP);
	Shaders::RenderToDepthBuffer(pd3dCommandList, pCamera);

	for (UINT j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] && !m_ppObjects[j]->isLive())
			continue;

		float distance = Vector3::Length(Vector3::Subtract(m_ppObjects[j]->GetPosition(), cameraPos, false));
		//if(distance <= offset)
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

void ModelShader::Animate(float fTimeElapsed)
{
	if (IsZero(fTimeElapsed))
		return;
	for (UINT i = 0; i < m_nObjects; ++i) {
		if(m_ppObjects[i]->isLive())
			m_ppObjects[i]->Animate(fTimeElapsed);
	}
}

void ModelShader::setScale(float scale)
{
	for (int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i]->SetScale(scale);
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
	int i = 0;
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, SRVTexture2D, 0, 0); // GameObject
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArray, 0, 0); // Texture
	
	CD3DX12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);

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


void DynamicModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
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

void DynamicModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	ModelLoader* globalModels = GlobalVal::getInstance()->getModelLoader();
	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSStaticModel", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));
	
	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	
	Player* tmp=new Player(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	tmp->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
	//tmp->SetPosition(XMFLOAT3(0, 0, 0));
	tmp->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	m_ppObjects[0] = tmp;
	
}

void DynamicModelShader::Animate(float fTimeElapsed)
{
	ModelShader::Animate(fTimeElapsed);
}

