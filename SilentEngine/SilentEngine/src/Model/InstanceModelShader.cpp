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
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Texture2DArray
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = 0;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; // InstanceData
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

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
	int nTextureType = pTexture->GetTextureType();

	d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;
	d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;

	for (int i = 0; i < nTextures; i++)
	{
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
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticInstanceModel", "vs_5_1");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSStaticInstanceModel", "ps_5_1");

	m_nObjects = 4;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalMaps->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalMaps->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, 1, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}

	for (UINT i = 0; i < m_nObjects; ++i) {
		ModelObject* object = new ModelObject(globalMaps->getModel(modelIndex), pd3dDevice, pd3dCommandList);
		object->SetScale(0.8f);
		object->SetPosition(0, -180, 0);
		object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));

		m_ppObjects[i] = object;
	}
}

void InstanceModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) 
		m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects,pCamera);
}

void InstanceModelShader::SetPhys(BasePhysX * phys)
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
	//m_ppObjects[0]->Rotate(0, 90, 0);
	//m_ppObjects[1]->Rotate(0, 90, 0);
	float rot[4] = { 90,90,0,0 };
	for (UINT i = 0; i < 4; ++i) {
		ModelObject* door = reinterpret_cast<ModelObject*>(m_ppObjects[i]);
		door->SetActorPos(pos[i].xPos, pos[i].yPos - 20.0f, pos[i].zPos, rot[i]);
	}
	//for (UINT i = 0; i < m_nObjects; ++i) {

	//	m_ppObjects[i]->SetPosition(pos[i].xPos,pos[i].yPos -20.0f,pos[i].zPos);
	//}
	
}

void MapShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSStaticInstanceModel", "vs_5_1");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSStaticInstanceModel", "ps_5_1");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalMaps->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalMaps->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, 1, false);

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

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

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

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

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
	int nTextureType = pTexture->GetTextureType();

	d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;
	d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize * nInstanceParameterCount;

	for (int i = 0; i < nTextures; i++)
	{
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
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicInstanceModel", "vs_5_1");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicInstanceModel", "ps_5_1");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_BoneCB->Resource(), 1, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, 1, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}

	int num = 0;
	ModelObject* object = new ModelObject(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	object->SetPosition(XMFLOAT3(0, 0, 0));
	object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * num));

	m_ppObjects[num] = object;
}

void InstanceDynamicModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::OnPrepareRender(pd3dCommandList);
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial)
		m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera);
}


