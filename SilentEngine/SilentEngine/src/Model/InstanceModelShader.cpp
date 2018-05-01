#include "stdafx.h"
#include "InstanceModelShader.h"

InstanceModelShader::InstanceModelShader(UINT index) :
	ModelShader(index)
{
}

InstanceModelShader::~InstanceModelShader()
{
}

void InstanceModelShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	int i = 0;
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3 + NUM_DIRECTION_LIGHTS];

	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArray, 0, 0); // GameObject
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVTexArrayNorm, 0, 0); // NormTexture
	pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVInstanceData, 0, 0); // Texture
	for (i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
		pd3dDescriptorRanges[3 + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9 + i, 0, 0); // ShadowMap

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[7 + NUM_DIRECTION_LIGHTS];

	pd3dRootParameters[0].InitAsConstantBufferView(1);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[2].InitAsConstantBufferView(4);
	pd3dRootParameters[3].InitAsConstantBufferView(5);
	pd3dRootParameters[4].InitAsConstantBufferView(8);
	pd3dRootParameters[5].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	pd3dRootParameters[6].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
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

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_SHADOWMAP].GetAddressOf()))
	);
}


void InstanceModelShader::CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource * pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement)
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

void InstanceModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	// Instance 데이터는 상수버퍼가 아닌 셰이더 리소스 버퍼에 저장되기 떄문에 마지막 인자를 false로 설정
	m_ObjectCB = make_unique<UploadBuffer<CB_GAMEOBJECT_INFO>>(pd3dDevice, m_nObjects, false);
}

void InstanceModelShader::CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement)
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

void InstanceModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
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

void InstanceModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	MapLoader* globalMaps = GlobalVal::getInstance()->getMapLoader();

	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticInstanceModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSStaticInstanceModel", "ps_5_0");

	m_nObjects = 4;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 3 + NUM_DIRECTION_LIGHTS);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalMaps->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalMaps->getMat(modelIndex).c_str(), 0);
		for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
			pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 6, 1, true);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}

	for (UINT i = 0; i < m_nObjects; ++i) {
		ModelObject* object = new ModelObject(globalMaps->getModel(modelIndex), pd3dDevice, pd3dCommandList);
		object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = object;
	}
}

void InstanceModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);
	Shaders::Render(pd3dCommandList, pCamera);

	pd3dCommandList->SetGraphicsRootConstantBufferView(4, m_FogCB->Resource()->GetGPUVirtualAddress());

	if (m_pMaterial) 
		m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects,pCamera);
}

void InstanceModelShader::setPhys(BasePhysX * phys)
{
	for (auto& p : m_ppObjects) {
		reinterpret_cast<ModelObject*>(p)->SetPhysMesh(phys, PhysMesh::Mesh_Tri);
	}
}

void InstanceModelShader::releasePhys()
{
	for (auto& p : m_ppObjects) {
		reinterpret_cast<ModelObject*>(p)->releasePhys();
	}
}

void InstanceModelShader::SetPositions(Point * pos)
{
	float rot[4] = { 90,90,0,0 };
	for (UINT i = 0; i < 4; ++i) {
		ModelObject* door = reinterpret_cast<ModelObject*>(m_ppObjects[i]);
		door->SetActorPos(pos[i].xPos, pos[i].yPos - 20.0f, pos[i].zPos, rot[i]);
	}
}

void InstanceModelShader::Animate(float fTime, UINT* next)
{
	//for gate
	float rot[4] = { -1,1,1,-1 };

	for (UINT i = 0; i < 4; ++i) {
		if (next[i] != 100) {
			ModelObject* door = reinterpret_cast<ModelObject*>(m_ppObjects[i]);
			door->RotationYAxis(rot[i]);
		}
	}
}

void MapShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	MapLoader* globalMaps = GlobalVal::getInstance()->getMapLoader();

	m_nPSO = 2;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticInstanceNORMModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\NormalMap.hlsl", nullptr, "PSModelNormalMap", "ps_5_0");
	
	m_VSByteCode[1] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticInstanceNORMModel", "vs_5_0");
	m_PSByteCode[1] = nullptr;

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 3 + NUM_DIRECTION_LIGHTS);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, false);

	CreateGraphicsRootSignature(pd3dDevice);

	BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
	BuildPSO(pd3dDevice, nRenderTargets);

	int index = 0;
	
	if (globalMaps->isMat(modelIndex)) {
		bool hadNormap = globalMaps->isNormat(modelIndex);
		CTexture *pTexture = new CTexture(hadNormap ? 2 : 1, RESOURCE_TEXTURE2D, 0);
		if (hadNormap)
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalMaps->getNorMat(modelIndex).c_str(), index++);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalMaps->getMat(modelIndex).c_str(), index);
	
		for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
			pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 6 - index, 1, true);
	
		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}
	
	int num = 0;
	ModelObject* object = new ModelObject(globalMaps->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	object->SetPosition(XMFLOAT3(0, 0, 0));
	object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * num));
	m_ppObjects[num] = object;
}

InstanceDynamicModelShader::InstanceDynamicModelShader(int index) :
	DynamicModelShader(index)
{
}

InstanceDynamicModelShader::~InstanceDynamicModelShader()
{
}

void InstanceDynamicModelShader::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Texture2DArray
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; // InstanceData
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 9; //Texture2DArray
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[6];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; // InstanceData
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
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //Texture
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


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
		IID_PPV_ARGS(m_RootSignature[PSO_OBJECT].GetAddressOf()))
	);

	ThrowIfFailed(pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature[PSO_SHADOWMAP].GetAddressOf()))
	);
}

void InstanceDynamicModelShader::CreateInstanceShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12Resource * pd3dConstantBuffers, UINT nRootParameterStartIndex, bool bAutoIncrement)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	for (int i = 0; i < 1; i++)
	{
		D3D12_RESOURCE_DESC d3dResourceDesc = pd3dConstantBuffers->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, RESOURCE_BUFFER);

		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_nObjects;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = sizeof(CB_DYNAMICOBJECT_INFO);

		pd3dDevice->CreateShaderResourceView(pd3dConstantBuffers, &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
	}
}

void InstanceDynamicModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_BoneCB = make_unique<UploadBuffer<CB_DYNAMICOBJECT_INFO>>(pd3dDevice, m_nObjects, false);
}

void InstanceDynamicModelShader::CreateShaderResourceViews(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, CTexture * pTexture, UINT nRootParameterStartIndex, UINT nInstanceParameterCount, bool bAutoIncrement)
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

void InstanceDynamicModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_DYNAMICOBJECT_INFO cBone;
	for (UINT i = 0; i < m_nObjects; ++i) {
		XMStoreFloat4x4(&cBone.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
		cBone.m_nMaterial = 0;

		memcpy(cBone.m_bone, m_ppObjects[i]->GetBoneData(), sizeof(XMFLOAT4X4) * m_ppObjects[i]->GetBoneNum());

		m_BoneCB->CopyData(i, cBone);
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(2, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, m_LightsCB->Resource()->GetGPUVirtualAddress());
}

void InstanceDynamicModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{

}

void InstanceDynamicModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial)
		m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0]) {
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera);
	}
}

void InstanceDynamicModelShader::RenderToDepthBuffer(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList, PSO_SHADOWMAP);
	Shaders::RenderToDepthBuffer(pd3dCommandList, pCamera);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera);
}