#include "stdafx.h"
#include "Scene.h"

Scene::Scene()
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

void TestScene::BuildBoxGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Geometries = std::make_shared<unordered_map<string, unique_ptr<MeshGeometry>>>();
	(*m_Geometries)["boxGeo"] = move(std::make_unique<MeshGeometryCube>(pDevice, pCommandList, 10.0f, 10.0f, 10.0f));
}

void TestScene::BuildRootSignature(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);	// Camera

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(pDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["VSCube"]->GetBufferPointer()),
		m_Shaders["VSCube"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["PSCube"]->GetBufferPointer()),
		m_Shaders["PSCube"]->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSOs["Cube"])));
}

void TestScene::BuildConstantBuffers(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<ObjectConstants>>(pDevice, 1, true);

	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();
	
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	pDevice->CreateConstantBufferView(
		&cbvDesc,
		m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void TestScene::Update(const Timer & gt)
{
}

void TestScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	
	m_pShaders = make_unique<Shaders>();
	for (int i = 0; i < 5; ++i)
		(m_pShaders.get())->BuildObjects(pDevice, pCommandList);
	//m_pShaders->BuildObjects(pDevice, pCommandList);

	//BuildDescriptorHeaps(pDevice, pCommandList);
	//BuildConstantBuffers(pDevice, pCommandList);
	//BuildRootSignature(pDevice, pCommandList);
	//BuildShadersAndInputLayout(pDevice, pCommandList);
	//BuildSceneGeometry(pDevice, pCommandList);
	//BuildPSOs(pDevice, pCommandList);

	

	m_Camera = make_unique<Camera>();
	m_Camera->InitCamera(pDevice, pCommandList);
}

void TestScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	//pCommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	//pCommandList->SetPipelineState(m_PSOs["Cube"].Get());
	
	//m_Camera->SetViewportsAndScissorRects(pCommandList);
	//m_Camera->UpdateShaderVariables(pCommandList);
		(m_pShaders.get())->Render(pCommandList, m_Camera.get());
	//m_pShaders->Render(pCommandList, m_Camera.get());
	
	//(*m_Geometries.get())["boxGeo"]->Render(pCommandList);
}
