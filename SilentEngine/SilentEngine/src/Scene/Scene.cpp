#include "stdafx.h"
#include "Scene.h"
#include "InstanceObjectShader.h"
#include "..\Model\ModelShader.h"

Scene::Scene() : m_physics(nullptr)
{
}

Scene::~Scene()
{
}

void Scene::BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
	BuildDescriptorHeaps(pDevice, pCommandList);
	BuildRootSignature(pDevice, pCommandList);
	BuildShadersAndInputLayout(pDevice, pCommandList);
	BuildPSOs(pDevice, pCommandList);
}

void Scene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	
}

TestScene::TestScene()
{
	m_physics = new BasePhysX(60.0f);
}

TestScene::~TestScene()
{
	delete m_physics;
}

void TestScene::BuildBoxGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Geometries = std::make_shared<unordered_map<string, unique_ptr<MeshGeometry>>>();
	(*m_Geometries)["boxGeo"] = move(std::make_unique<MeshGeometryCube>(pDevice, pCommandList, 10.0f, 10.0f, 10.0f));
}

void TestScene::BuildRootSignature(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2; //Game Objects
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
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Game Objects
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
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //Texture2DArray
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

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	ThrowIfFailed(pDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}

void TestScene::BuildDescriptorHeaps(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(pDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_SrvDescriptorHeap)));
}

void TestScene::BuildShadersAndInputLayout(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	HRESULT hr = S_OK;

	m_Shaders["VSCube"] = COMPILEDSHADERS->GetCompiledShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_Shaders["PSCube"] = COMPILEDSHADERS->GetCompiledShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void TestScene::BuildSceneGeometry(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	BuildBoxGeometry(pDevice, pCommandList);
}

void TestScene::BuildPSOs(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	
}

void TestScene::CreateShaderVariables(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pd3dcbLights = make_unique<UploadBuffer<LIGHTS>>(pDevice, 1, true);
	m_pd3dcbMaterials = make_unique<UploadBuffer<MATERIALS>>(pDevice, 1, true);
}

void TestScene::UpdateShaderVarialbes() {
	m_pd3dcbLights->CopyData(0, *m_pLights);
	m_pd3dcbMaterials->CopyData(0, *m_pMaterials);
}

void TestScene::Update(const Timer & gt)
{
	m_physics->stepPhysics(false);
	for (UINT i = 0; i < m_nShaders; ++i) {
		m_ppShaders[i]->Animate(gt.DeltaTime());
	}
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{

	BuildRootSignature(pDevice, pCommandList);

	m_nShaders = 3;
	m_ppShaders = new Shaders*[m_nShaders];
	
	IlluminatedObjectShader* pIlluminatedObject = new IlluminatedObjectShader();
	pIlluminatedObject->BuildObjects(pDevice, pCommandList);
	pIlluminatedObject->BuildPSO(pDevice, m_RootSignature.Get());
	m_ppShaders[0] = pIlluminatedObject;

	ModelShader* tmp= new ModelShader(2);
	tmp->setPhysics(m_physics);
	m_ppShaders[1] = tmp;
	
	DynamicModelShader* tmp2 = new DynamicModelShader(1);
	tmp2->setPhysics(m_physics);
	m_ppShaders[2] = tmp2;

	for(UINT i=0; i<m_nShaders; ++i)
		m_ppShaders[i]->BuildObjects(pDevice, pCommandList);

	BuildLightsAndMaterials();
	CreateShaderVariables(pDevice, pCommandList);

	m_Camera = make_unique<Camera>();
	m_Camera->InitCamera(pDevice, pCommandList);
}

void TestScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	UpdateShaderVarialbes();

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->Resource()->GetGPUVirtualAddress();
	pCommandList->SetGraphicsRootConstantBufferView(2, d3dcbMaterialsGpuVirtualAddress); //Materials

	for(UINT i=0; i<m_nShaders; ++i)
		m_ppShaders[i]->Render(pCommandList, m_Camera.get());
}

bool TestScene::OnKeyboardInput(const Timer& gt, UCHAR *pKeysBuffer)
{
	return false;
}

bool TestScene::OnMouseDown(HWND& hWin, WPARAM btnState, int x, int y)
{
	return false;
}

bool TestScene::OnMouseUp(HWND& hWin, WPARAM btnState, int x, int y)
{
	return false;
}

bool TestScene::OnMouseMove(HWND& hWin, WPARAM btnState, int x, int y)
{
	return false;
}

void TestScene::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS();
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = true;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = true;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 50.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 60.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(-150.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 8.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pMaterials = new MATERIALS();
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}