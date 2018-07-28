#include "stdafx.h"
#include "UIShaders.h"
#include "GameObjects.h"
#include "..\Room\Room.h"

UIShaders::UIShaders() { }
UIShaders::~UIShaders() { }

D3D12_BLEND_DESC UIShaders::CreateBlendState(int index)
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

D3D12_DEPTH_STENCIL_DESC UIShaders::CreateDepthStencilState(int index)
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

void UIShaders::Animate(float fTimeElapsed)
{
	for (int i = 0; i < m_nObjects; ++i)
		m_pUIObjects[i]->Update(fTimeElapsed);
}

void UIShaders::RefreshTimer(float fTimeElapsed, UINT index)
{
	if (!m_pUIObjects[index]->m_bEnabled) return;

	m_ElapsedTime += fTimeElapsed;

	if (m_ElapsedTime >= 0.0f && m_ElapsedTime <= m_ApearTime) 
		m_pUIObjects[index]->SetAlpha(D3DMath::Lerp(0.0f, 1.0f, m_ElapsedTime / m_ApearTime));
		
	else if (m_ElapsedTime >= m_ApearTime && m_ElapsedTime <= m_ApearTime + m_ExistTime) 
		m_pUIObjects[index]->SetAlpha(1.0f);
	
	else if (m_ElapsedTime >= m_ApearTime + m_ExistTime && m_ElapsedTime <= m_ApearTime + m_ExistTime + m_RetreatTime)
		m_pUIObjects[index]->SetAlpha(D3DMath::Lerp(1.0f, 0.0f, ((m_ElapsedTime - (m_ApearTime + m_ExistTime)) / m_RetreatTime)));

	else 
		m_pUIObjects[index]->m_bEnabled = false;
	
	return;
}

void UIShaders::SetNowSprite(XMUINT2 & nowSprite, UINT index)
{
	m_pUIObjects[index]->m_nNowSprite = nowSprite;
	if (m_bTimeUI) {
		m_pUIObjects[index]->m_bEnabled = true;
		m_ElapsedTime = 0.0f;
	}
}

void UIShaders::SetPosScreenRatio(XMFLOAT2& ratio, UINT index)
{
	m_pUIObjects[index]->m_xmf2ScreenPos = XMFLOAT2(
		static_cast<float>(FRAME_BUFFER_WIDTH) * ratio.x,
		static_cast<float>(FRAME_BUFFER_HEIGHT) * ratio.y
	);

	m_pUIObjects[index]->CreateCollisionBox();
}

void UIShaders::SetPosScreenRatio(XMFLOAT2& ratio, const OPTIONSETALL)
{
	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->m_xmf2ScreenPos = XMFLOAT2(
			static_cast<float>(FRAME_BUFFER_WIDTH) * ratio.x,
			static_cast<float>(FRAME_BUFFER_HEIGHT) * ratio.y
		);

		m_pUIObjects[i]->CreateCollisionBox();
	}
}

void UIShaders::SetAlpha(float alpha, UINT index)
{
	m_pUIObjects[index]->m_fAlpha = alpha;
}

void UIShaders::SetScale(XMFLOAT2 * scale, const OPTIONSETALL)
{
	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->m_xmf2Scale = *scale;
	}

	CreateCollisionBox();
}

void UIShaders::MovePos(XMFLOAT2 & pos, UINT index)
{
	XMFLOAT2 originPos = m_pUIObjects[index]->m_xmf2ScreenPos;
	XMFLOAT2 originScale = m_pUIObjects[index]->m_xmf2Scale;
	m_pUIObjects[index]->SetPosition(XMFLOAT2(originPos.x + (pos.x * originScale.x), originPos.y + (pos.y * originScale.y)));
}

void UIShaders::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1 + NUM_MAX_UITEXTURE];
	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CBVUIInfo, 0, 0); // GameObject
	for (int i = 0; i < NUM_MAX_UITEXTURE; ++i)
		pd3dDescriptorRanges[1 + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVUITextureMap + i, 0, 0); // Texture

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[1 + NUM_MAX_UITEXTURE];
	pd3dRootParameters[0].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	for (int i = 0; i < NUM_MAX_UITEXTURE; ++i)
		pd3dRootParameters[1 + i].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1 + i], D3D12_SHADER_VISIBILITY_ALL);

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

void UIShaders::CreateCollisionBox()
{
	for (int i = 0; i < m_nObjects; ++i)
		m_pUIObjects[i]->CreateCollisionBox();
}

void UIShaders::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<CB_UI_INFO>>(pd3dDevice, m_nObjects, true);
}

void UIShaders::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_UI_INFO cBuffer;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		cBuffer.m_fData = m_pUIObjects[i]->m_fData;
		cBuffer.m_fData2 = m_pUIObjects[i]->m_fData2;
		cBuffer.m_nNowSprite = m_pUIObjects[i]->m_nNowSprite;
		cBuffer.m_nNumSprite = m_pUIObjects[i]->m_nNumSprite;
		cBuffer.m_nSize = m_pUIObjects[i]->m_nSize;
		cBuffer.m_xmf2Scale = m_pUIObjects[i]->m_xmf2Scale;
		cBuffer.m_nTexType = m_pUIObjects[i]->m_nTexType;
		cBuffer.m_xmf2ScreenPos = m_pUIObjects[i]->m_xmf2ScreenPos;
		cBuffer.m_xmf2ScreenSize = m_pUIObjects[i]->m_xmf2ScreenSize;
		cBuffer.m_fAlpha = m_pUIObjects[i]->m_fAlpha;
		m_ObjectCB->CopyData(i, cBuffer);
	}
}

void UIShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) { 
	TextureDataForm* textures = reinterpret_cast<TextureDataForm*>(pContext);
	UINT nTextures = 1;

	m_nObjects = nTextures;
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSDefaultUI", "ps_5_0");

	CTexture *pTexture = new CTexture(nTextures, RESOURCE_TEXTURE2D, 0);
	for (int i = 0; i < nTextures; ++i) {
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (*textures).m_texture.c_str(), i);
	}
	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 scale = XMFLOAT2(0.7f, 0.7f);

	for (int i = 0; i < m_nObjects; ++i) {
		UIObject* ui;
		ui = new UIObject();
		ui->SetPosition(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH) / 2, static_cast<float>(FRAME_BUFFER_HEIGHT) * (3.0f - 1.5f * i) / 9.0));
		ui->SetScale(scale);
		m_pUIObjects[i] = ui;
	}

	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetNumSprite(XMUINT2((*textures).m_MaxX, (*textures).m_MaxY), XMUINT2(0, 0));
		m_pUIObjects[i]->SetSize(GetSpriteSize(i, pTexture, XMUINT2(XMUINT2((*textures).m_MaxX, (*textures).m_MaxY))));
		m_pUIObjects[i]->SetType(i);
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}
}

void UIShaders::OnPrepareRender(ID3D12GraphicsCommandList * pd3dCommandList, int index)
{
	if (m_RootSignature[index])
		pd3dCommandList->SetGraphicsRootSignature(m_RootSignature[index].Get());

	if (m_pPSO[index])
		pd3dCommandList->SetPipelineState(m_pPSO[index].Get());

	pd3dCommandList->SetDescriptorHeaps(1, m_CbvSrvDescriptorHeap.GetAddressOf());

	UpdateShaderVariables(pd3dCommandList);
}

void UIShaders::Render(ID3D12GraphicsCommandList * pd3dCommandList)
{
	OnPrepareRender(pd3dCommandList, 0);
	
	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	for (unsigned int j = 0; j < m_nObjects; j++)
	{
		if (m_pUIObjects[j] && m_pUIObjects[j]->m_bEnabled)
			m_pUIObjects[j]->Render(pd3dCommandList);
	}
}

void UIShaders::SetTimer(float apearTime, float existTime, float retreatTime)
{
	m_bTimeUI		= true;
	m_ApearTime		= apearTime;
	m_RetreatTime	= retreatTime;
	m_ExistTime		= existTime;
	m_ElapsedTime	= 0.0f;
}

XMUINT2 UIShaders::GetSpriteSize(const int texIndex, CTexture* pTexture, XMUINT2& numSprite)
{
	D3D12_RESOURCE_DESC desc = pTexture->GetTexture(texIndex)->GetDesc();
	return XMUINT2(desc.Width / numSprite.x, desc.Height / numSprite.y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void UIHPBarShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 3;
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSUIHPBar", "ps_5_0");

	CTexture *pTexture = new CTexture(m_nObjects, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\HPBarTedori.dds", 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\HPBar.dds", 1);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\MPBar.dds", 2);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 pos = XMFLOAT2(
		static_cast<float>(FRAME_BUFFER_WIDTH) / 6.0,
		static_cast<float>(FRAME_BUFFER_HEIGHT) * 8.0f / 9.0
	);
	XMFLOAT2 scale = XMFLOAT2(0.8f, 0.8f);
	XMUINT2 hpTedoriSize = GetSpriteSize(0, pTexture, XMUINT2(1, 1));

	UIObject* HPTedori;
	HPTedori = new UIObject();
	HPTedori->SetPosition(pos);
	HPTedori->SetScale(scale);
	m_pUIObjects[0] = HPTedori;

	pos = XMFLOAT2(
		pos.x + static_cast<float>(hpTedoriSize.x) * (0.65f - 0.5f) * scale.x, 
		pos.y + static_cast<float>(hpTedoriSize.y) * (0.272f) * scale.y
	);

	HPBarObject* hpBar;
	hpBar = new HPBarObject();
	hpBar->SetPosition(XMFLOAT2(pos));
	hpBar->SetPlayerStatus(reinterpret_cast<GameObject*>(pContext)->GetStatus());
	hpBar->SetScale(scale);
	m_pUIObjects[1] = hpBar;

	pos = XMFLOAT2(
		pos.x,
		pos.y - static_cast<float>(hpTedoriSize.y) * (0.161f) * scale.y
	);

	HPBarObject* mpBar;
	mpBar = new HPBarObject();
	mpBar->SetPosition(pos);
	mpBar->SetHPType(false);
	mpBar->SetPlayerStatus(reinterpret_cast<GameObject*>(pContext)->GetStatus());
	mpBar->SetScale(scale);
	m_pUIObjects[2] = mpBar;

	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetSize(GetSpriteSize(i, pTexture, XMUINT2(1, 1)));
		m_pUIObjects[i]->SetType(i);
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}

}

//

void UIMiniMapShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nPSO = 1;
	Room** data = reinterpret_cast<Room**>(pContext);

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSMiniMap", "ps_5_0");

	pTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\Box_COLOR.dds", 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\minimapTedori.dds", 1);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	//m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

}

void UIMiniMapShaders::Animate(float fTimeElapsed)
{
	if (m_pPreRoom != *m_pNowRoom + 1) {
		m_pUIObjects[m_pPreRoom]->m_fData = 2.0f;
		m_pUIObjects[*m_pNowRoom + 1]->m_fData = 1.0f;
		m_pPreRoom = *m_pNowRoom + 1;
	}
}

void UIMiniMapShaders::setRoomPos(void* pContext)
{
	Room** data = reinterpret_cast<Room**>(pContext);

	m_pUIObjects.clear();
	m_pUIObjects.resize(m_nObjects);
	//m_pUIObjects = vector<UIObject*>(m_nObjects);
	UIObject* minimapBG = new UIObject();
	XMFLOAT2 sacle = XMFLOAT2(1.1f,0.5f);

	minimapBG->SetType(1);
	minimapBG->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
	minimapBG->SetNumSprite(XMUINT2(1, 1), XMUINT2(0, 0));
	minimapBG->SetScale(sacle);
	minimapBG->SetSize(GetSpriteSize(1, pTexture, minimapBG->m_nNumSprite));
	minimapBG->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr);
	minimapBG->SetPosition(XMFLOAT2(
		static_cast<float>(FRAME_BUFFER_WIDTH) * 7.05 / 8.0,
		static_cast<float>((FRAME_BUFFER_HEIGHT) * 7.0 / 8.0)
	));
	m_pUIObjects[0] = minimapBG;
	m_pUIObjects[0]->CreateCollisionBox();

	sacle.y = sacle.x;
	for (unsigned int i = 1; i < m_nObjects; ++i) {
		UIObject* minimapObj = new UIObject();

		minimapObj->SetType(0);
		minimapObj->SetScale(sacle);
		minimapObj->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		minimapObj->SetNumSprite(XMUINT2(3, 1), XMUINT2(0, 0));
		minimapObj->SetSize(GetSpriteSize(0, pTexture, minimapObj->m_nNumSprite));
		minimapObj->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		minimapObj->SetPosition(XMFLOAT2(
			static_cast<float>(FRAME_BUFFER_WIDTH) * 6.6 / 8.0 + static_cast<float>(data[i - 1]->m_mapPosX * minimapObj->m_nSize.x),
			static_cast<float>((FRAME_BUFFER_HEIGHT) * 7.3 / 8.0 - data[i - 1]->m_mapPosY * minimapObj->m_nSize.y)
		));
		m_pUIObjects[i] = minimapObj;
		m_pUIObjects[i]->CreateCollisionBox();
	}
	m_pUIObjects[1]->m_fData = 1.0f;
	m_pPreRoom = 1;
}

void UIButtonShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	vector<TextureDataForm>* textures = reinterpret_cast<vector<TextureDataForm>*>(pContext);
	UINT nTextures = textures->size();

	m_nObjects = nTextures;
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSMiniMap", "ps_5_0");

	CTexture *pTexture = new CTexture(nTextures, RESOURCE_TEXTURE2D, 0);
	for (int i = 0; i < nTextures; ++i) {
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, (*textures)[i].m_texture.c_str(), i);
	}
	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 scale = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < m_nObjects; ++i) {
		UIObject* button1;
		button1 = new UIObject();
		button1->SetPosition(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH) / 2, static_cast<float>(FRAME_BUFFER_HEIGHT) * (3.0f - 1.5f * i) / 9.0));
		button1->SetScale(scale);
		m_pUIObjects[i] = button1;
	}

	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetNumSprite(XMUINT2((*textures)[i].m_MaxX, (*textures)[i].m_MaxY), XMUINT2(0, 0));
		m_pUIObjects[i]->SetSize(GetSpriteSize(i, pTexture, XMUINT2((*textures)[i].m_MaxX, (*textures)[i].m_MaxY)));
		m_pUIObjects[i]->SetType(i);
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}
}

UINT UIButtonShaders::CollisionButton()
{
	UINT collision = 0;
	XMFLOAT2 trueSetData = XMFLOAT2();
	XMFLOAT2 falseSetData = XMFLOAT2();
	for (UINT i = 0; i < m_nObjects; ++i) {
		trueSetData = XMFLOAT2( 1.0f, m_pUIObjects[i]->m_fData2);
		falseSetData = XMFLOAT2( 0.0f, m_pUIObjects[i]->m_fData2);

		if(m_pUIObjects[i]->CollisionUI(m_pMousePosition, trueSetData, falseSetData))
			collision = i + 1;
	}
	return collision;
}

UINT UIButtonShaders::ClickButton()
{
	UINT collision = 0;
	XMFLOAT2 trueSetData = XMFLOAT2();
	XMFLOAT2 falseSetData = XMFLOAT2();
	for (UINT i = 0; i < m_nObjects; ++i) {
		UINT newYData = static_cast<UINT>((m_pUIObjects[i]->m_fData2) + 1) % m_pUIObjects[i]->m_nNumSprite.y;
		trueSetData = XMFLOAT2(1.0f, static_cast<float>(newYData));
		falseSetData = XMFLOAT2(0.0f, m_pUIObjects[i]->m_fData2);

		if (m_pUIObjects[i]->CollisionUI(m_pMousePosition, trueSetData, falseSetData))
			collision = i + 1;
	}
	return collision;
}

/////////////////////////////////////////////////////////////////////////////////////

void UIFullScreenShaders::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets, void *pContext)
{
	TextureDataForm* textures = reinterpret_cast<TextureDataForm*>(pContext);
	m_nObjects = 2;
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSBlockUI", "ps_5_0");

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, textures->m_texture.c_str(), 0);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 pos[2] = { XMFLOAT2(
			static_cast<float>(FRAME_BUFFER_WIDTH) * 0.25,
			static_cast<float>(FRAME_BUFFER_HEIGHT) * 0.5f
		),
			XMFLOAT2(
			static_cast<float>(FRAME_BUFFER_WIDTH) * 0.75,
			static_cast<float>(FRAME_BUFFER_HEIGHT) * 0.5f
		) 
	};
	XMFLOAT2 scale = XMFLOAT2(0.5f, 1.0f);

	UIObject* leftBlock;
	leftBlock = new UIObject();
	leftBlock->SetPosition(pos[0]);
	leftBlock->SetScale(scale);
	m_pUIObjects[0] = leftBlock;

	UIObject* rightBlock;
	rightBlock = new UIObject();
	rightBlock->SetPosition(pos[1]);
	rightBlock->SetScale(scale);
	m_pUIObjects[1] = rightBlock;


	for (int i = 0; i < m_nObjects; ++i) {
		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetSize(GetSpriteSize(0, pTexture, XMUINT2(1, 1)));
		m_pUIObjects[i]->SetType(0);
		m_pUIObjects[i]->m_nNumSprite = XMUINT2(2, 1);
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_pUIObjects[i]->m_fData = i * 1.0f;
	}

}

void SkillUIShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	TextureDataForm* textures = reinterpret_cast<TextureDataForm*>(pContext);
	m_nObjects = textures->m_MaxX;	// 최대 쿨타임 개수
	m_pCooldown.resize(m_nObjects);
	m_pSkillTime.resize(m_nObjects);
	m_pBonus.resize(m_nObjects);

	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSCoolDownUI", "ps_5_0");

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, textures->m_texture.c_str(), 0);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 1, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 scale = XMFLOAT2(1.1f, 1.0f);

	for (int i = 0; i < m_nObjects; ++i) {

		UIObject* cooldownUI;
		cooldownUI = new UIObject();
		cooldownUI->SetPosition(XMFLOAT2(100.0f * (i + 1) , 100.0f * (i + 1)));
		cooldownUI->SetScale(scale);
		m_pUIObjects[i] = cooldownUI;

		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetSize(GetSpriteSize(0, pTexture, XMUINT2(m_nObjects, 1)));
		m_pUIObjects[i]->SetType(0);
		m_pUIObjects[i]->SetNumSprite(XMUINT2(m_nObjects, 1), XMUINT2(i, 1));	
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}
}

void SkillUIShaders::Animate(float fTimeElapsed)
{
	float elapsedTime = 0.0f;
	float coolTime = 0.0f;

	for (int i = 0; i < m_nObjects; ++i) {
		if (m_pCooldown[i] > GetTickCount() - *m_pSkillTime[i]) {
			m_pUIObjects[i]->m_bEnabled = true;

			elapsedTime = static_cast<float>(GetTickCount() - *m_pSkillTime[i]);
			coolTime = static_cast<float>(m_pCooldown[i]);				
			coolTime -= (m_pBonus[i] == nullptr) ? 0.0f : *m_pBonus[i];	// 쿨타임 관련 보너스가 연결 되어 있으면 연결

			m_pUIObjects[i]->m_fData = elapsedTime  / coolTime;
			if(elapsedTime / coolTime >= 1.0f)
				m_pUIObjects[i]->m_bEnabled = false;
		}
		else
			m_pUIObjects[i]->m_bEnabled = false;
	}
}

void SkillUIShaders::LinkedSkillTime(DWORD * time, DWORD cooldown, DWORD* bonus, UINT index)
{
	m_pCooldown[index] = cooldown;
	m_pSkillTime[index] = time;
	m_pBonus[index] = bonus;
}

void NumberUIShaders::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	TextureDataForm* textures = reinterpret_cast<TextureDataForm*>(pContext);
	m_nObjects = 1;	// 오브젝트 개수
	
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "VSUITextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\UIShader.hlsl", nullptr, "PSNumberUI", "ps_5_0");

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, textures->m_texture.c_str(), 0);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_UI_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, false);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pUIObjects = vector<UIObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	XMFLOAT2 scale = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < m_nObjects; ++i) {

		UIObject* numberUI;
		numberUI = new UIObject();
		numberUI->SetPosition(XMFLOAT2(100.0f * (i + 1), 100.0f * (i + 1)));
		numberUI->SetScale(scale);
		m_pUIObjects[i] = numberUI;

		m_pUIObjects[i]->SetScreenSize(XMFLOAT2(static_cast<float>(FRAME_BUFFER_WIDTH), static_cast<float>(FRAME_BUFFER_HEIGHT)));
		m_pUIObjects[i]->SetSize(GetSpriteSize(0, pTexture, XMUINT2(14, 1)));
		m_pUIObjects[i]->SetType(0);
		m_pUIObjects[i]->SetNumSprite(XMUINT2(14, 1), XMUINT2(0, 1));
		m_pUIObjects[i]->CreateCollisionBox();
		m_pUIObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}
}


void NumberUIShaders::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	m_ObjectCB = make_unique<UploadBuffer<CB_UI_INFO>>(pd3dDevice, m_nObjects, true);
	m_NumberInfoCB = make_unique<UploadBuffer<CB_NUMBER_INFO>>(pd3dDevice, m_nObjects, true);
}

void NumberUIShaders::CreateGraphicsRootSignature(ID3D12Device * pd3dDevice)
{
	ComPtr<ID3D12RootSignature> pd3dGraphicsRootSignature = nullptr;

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1 + NUM_MAX_UITEXTURE];
	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CBVUIInfo, 0, 0); // GameObject
	for (int i = 0; i < NUM_MAX_UITEXTURE; ++i)
		pd3dDescriptorRanges[1 + i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SRVUITextureMap + i, 0, 0); // Texture

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[2 + NUM_MAX_UITEXTURE];
	pd3dRootParameters[0].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	pd3dRootParameters[1].InitAsConstantBufferView(CBVNumberUI);
	for (int i = 0; i < NUM_MAX_UITEXTURE; ++i)
		pd3dRootParameters[2 + i].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1 + i], D3D12_SHADER_VISIBILITY_ALL);

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


void NumberUIShaders::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_UI_INFO cBuffer;

	float data1, data2 = 0.0f;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		cBuffer.m_fData = m_pUIObjects[i]->m_fData;
		cBuffer.m_fData2 = m_pUIObjects[i]->m_fData2;
		cBuffer.m_nNowSprite = m_pUIObjects[i]->m_nNowSprite;
		cBuffer.m_nNumSprite = m_pUIObjects[i]->m_nNumSprite;
		cBuffer.m_nSize = m_pUIObjects[i]->m_nSize;
		cBuffer.m_xmf2Scale = m_pUIObjects[i]->m_xmf2Scale;
		cBuffer.m_nTexType = m_pUIObjects[i]->m_nTexType;
		cBuffer.m_xmf2ScreenPos = m_pUIObjects[i]->m_xmf2ScreenPos;
		cBuffer.m_xmf2ScreenSize = m_pUIObjects[i]->m_xmf2ScreenSize;
		cBuffer.m_fAlpha = m_pUIObjects[i]->m_fAlpha;
		m_ObjectCB->CopyData(i, cBuffer);
	}

	if (m_pData1 != nullptr) {
		data1 = (m_NumberInfo.m_Type < 3) ? *reinterpret_cast<float*>(m_pData1) : static_cast<float>(*reinterpret_cast<UINT*>(m_pData1));
	}
	else
		data1 = m_NumberInfo.m_fOriginNumber1;

	if (m_pData2 != nullptr) {
		data2 = (m_NumberInfo.m_Type < 3) ? *reinterpret_cast<float*>(m_pData2) : static_cast<float>(*reinterpret_cast<UINT*>(m_pData2));
	}
	else
		data2 = m_NumberInfo.m_fOriginNumber2;


	// 0 : float, 1 : float 분수 , 2 : float 퍼센트 , 3 : int , 4 : int 분수 , 5 : int 퍼센트
	switch (m_NumberInfo.m_Type) {
	case NUM_TYPE_FLOAT_NONE:
		ConvertOptNoneToUINTArray(data1, m_NumberInfo.m_MaximumLength, true);
		break;
	case NUM_TYPE_FLOAT_DIVISION:
		ConvertOptDivisionToUINTArray(data1, data2, m_NumberInfo.m_MaximumLength, true);
		break;
	case NUM_TYPE_FLOAT_PERCENTAGE:
		break;
	case NUM_TYPE_UINT_NONE:
		ConvertOptNoneToUINTArray(data1, m_NumberInfo.m_MaximumLength, false);
		break;
	case NUM_TYPE_UINT_DIVISION:
		ConvertOptDivisionToUINTArray(data1, data2, m_NumberInfo.m_MaximumLength, false);
		break;
	case NUM_TYPE_UINT_PERCENTAGE:
		break;
	}
	
	m_NumberInfoCB->CopyData(0, m_NumberInfo);
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, m_NumberInfoCB->Resource()->GetGPUVirtualAddress());
}

void NumberUIShaders::SetData(float data1, float data2)
{
	if (!data1 < 0.0f) m_NumberInfo.m_fOriginNumber1 = data1;
	if (!data2 < 0.0f) m_NumberInfo.m_fOriginNumber2 = data2;
}

void NumberUIShaders::LinkData(void * data1, void * data2)
{
	m_pData1 = data1;
	m_pData2 = data2;
}

void NumberUIShaders::SetNumberInfo(UINT type, UINT length)
{
	m_NumberInfo.m_Type = type;
	m_NumberInfo.m_MaximumLength = length;
}

void NumberUIShaders::ConvertOptNoneToUINTArray(float data, UINT nLength, bool isFloat)
{
	UINT udata = static_cast<UINT>(data * 10);
	int length = isFloat ? nLength - 2 : nLength;
	int frontZero = 0;
	for (int i = 0; i < length; ++i) {
		UINT unit = pow(10, length - i);
		UINT digit = udata / unit;
		if (digit == 0 && i == frontZero && i != length - 1) {
			m_NumberInfo.m_fNumber[i] = VOIDDIGIT;
			frontZero++;
		}
		else
			m_NumberInfo.m_fNumber[i] = digit;

		udata -= digit * unit;
	}
	
	if (isFloat) {
		m_NumberInfo.m_fNumber[nLength - 2] = POINT;
		m_NumberInfo.m_fNumber[nLength - 1] = udata;
	}
}

void NumberUIShaders::ConvertOptDivisionToUINTArray(float data1, float data2, UINT nLength, bool isFloat)
{
	UINT half = nLength / 2;
	UINT udata1 = static_cast<UINT>(data1 * 10);
	UINT udata2 = static_cast<UINT>(data2 * 10);

	int length = half;
	int frontZero = 0;

	for (int i = 0; i < length; ++i) {
		UINT unit = pow(10, length - i);
		UINT digit = udata1 / unit;
		if (digit == 0 && i == frontZero && i != length - 1) {
			m_NumberInfo.m_fNumber[i] = VOIDDIGIT;
			frontZero++;
		}
		else
			m_NumberInfo.m_fNumber[i] = digit;

		udata1 -= digit * unit;
	}

	m_NumberInfo.m_fNumber[length] = DIVISION;

	frontZero = 0;

	for (int i = 0; i < length; ++i) {
		UINT unit = pow(10, length - i);
		UINT digit = udata2 / unit;
		if (digit == 0 && i == frontZero) {
			m_NumberInfo.m_fNumber[i + length + 1] = VOIDDIGIT;
			frontZero++;
		}
		else
			m_NumberInfo.m_fNumber[i + length + 1] = digit;

		udata2 -= digit * unit;
	}
}
