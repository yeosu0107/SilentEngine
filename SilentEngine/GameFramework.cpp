#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	//���� �����ӿ�ũ ����
	m_pScene = NULL;
	m_hInstance = NULL;
	m_hWnd = NULL;

	_tcscpy_s(m_pszFrameRate, _T("SileEngine ("));


	//d3d ����
	pdxgiSwapChain = NULL;
	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	//pd3dPipelineState = NULL; 
	m_pd3dCommandList = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		m_ppd3dRenderTargetBuffers[i] = NULL;
	m_pd3dRtvDescriptorHeap = NULL;
	m_nRtvDescriptorIncrementSize = 0;
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDsvDescriptorHeap = NULL;
	m_nDsvDescriptorIncrementSize = 0;
	m_nSwapChainBufferIndex = 0;
	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i<m_nSwapChainBuffers; ++i)
		m_nFenceValue[i] = 0;


	client_width = FRAME_BUFFER_WIDTH;
	client_height = FRAME_BUFFER_HEIGHT;
}

CGameFramework::~CGameFramework()
{

}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	srand(timeGetTime());

	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//D3d ����
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();



	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//������Ʈ ����

	BuildObjects();

	return(true);
}

//����ü��
void CGameFramework::CreateSwapChain() {
	RECT rcClient; ::GetClientRect(m_hWnd, &rcClient);
	client_width = rcClient.right - rcClient.left;
	client_height = rcClient.bottom - rcClient.top;
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferDesc.Width = client_width;
	dxgiSwapChainDesc.BufferDesc.Height = client_height;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;

	dxgiSwapChainDesc.SampleDesc.Count = (bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (bMsaa4xEnable) ? (nMsaa4xQualityLevels - 1) : 0;

	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;

#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS 
	//�ĸ������ ũ�⸦ ����ȭ�� ũ��� �����Ѵ�.
	dxgiSwapChainDesc.Flags = 0;
#else 
	// ����ȭ���� �ػ󵵸� �ĸ������ ũ�⿡ �°� �����Ѵ�. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	//����ü���� �����Ѵ�. 
	HRESULT hResult = pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc,
		(IDXGISwapChain **)&pdxgiSwapChain);
	//Alt + Enter Ű�� ������ ��Ȱ��ȭ
	pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//����ü���� ���� �ĸ���� �ε����� �����Ѵ�.
	m_nSwapChainBufferIndex = pdxgiSwapChain->GetCurrentBackBufferIndex();
}

//d3d12 ����̽� ����
void CGameFramework::CreateDirect3DDevice() {
#if defined(_DEBUG) 
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&m_pd3dDebugController);
	m_pd3dDebugController->EnableDebugLayer();
#endif
	//DXGI ���丮�� �����Ѵ�.
	//::CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void **)&pdxgiFactory);
	::CreateDXGIFactory1(IID_PPV_ARGS(&pdxgiFactory));
	IDXGIAdapter1 *pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++) {
		//��͸� �����Ѵ�.
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		//����� �Ӽ��� �о�´�.
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue; //�÷��׸� Ȯ��, ����Ʈ���������� �۵��ϸ� ���� ��ͷ� �Ѿ��.
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pd3Device))))
			break; //�÷��װ� ����Ʈ���������� �۵����� ���� �� 
				   //�� ��͸� �̿� Ư������ 12.0�� �����ϴ� 
				   //�ϵ���� ����̽��� �����ϰ� �������θ� Ȯ��
	}
	if (!pd3dAdapter) {
		//Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� 
		//������ �� ������ WARP ����̽��� �����Ѵ�.
		pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&pd3Device);
	}
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x ���� ���ø� 
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	pd3Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�. 
	bMsaa4xEnable = (nMsaa4xQualityLevels > 1) ? true : false;

	//�潺�� �����ϰ� �潺 ���� 1�� �����Ѵ�.
	pd3Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);


	if (pd3dAdapter)
		pd3dAdapter->Release();
}

//Ŀ�ǵ�ť ����
void CGameFramework::CreateCommandQueueAndList() {
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//����(Direct) ��� ť�� �����Ѵ�.
	HRESULT hResult = pd3Device->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);
	//����(Direct) ��� �Ҵ��ڸ� �����Ѵ�.
	hResult = pd3Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator),
		(void **)&m_pd3dCommandAllocator);
	//����(Direct) ��� ����Ʈ�� �����Ѵ�.
	hResult = pd3Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator,
		NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close(); //��� ����Ʈ�� �����Ǹ� ����(Open) �����̹Ƿ� ����(Closed) ���·� �����. 
}

//������
void CGameFramework::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�. 
	HRESULT hResult = pd3Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.
	m_nRtvDescriptorIncrementSize = pd3Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�. 
	hResult = pd3Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	//����-���ٽ� ������ ���� ������ ũ�⸦ �����Ѵ�. 
	m_nDsvDescriptorIncrementSize = pd3Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}


//����Ÿ�ٺ� ����
void CGameFramework::CreateRenderTargetView() {
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		HRESULT hResult = pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dRenderTargetBuffers[i]);
		pd3Device->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

//�������ĽǺ� ����
void CGameFramework::CreateDepthStencilView() {
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = client_width;
	d3dResourceDesc.Height = client_height;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (bMsaa4xEnable) ? (nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	//����-���ٽ� ���۸� �����Ѵ�.
	pd3Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//����-���ٽ� ���� �並 �����Ѵ�. 
	pd3Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);

}

//����� ��������
void CGameFramework::OnResizeBackBuffers() {
	WaitForGpuComplete();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dDepthStencilBuffer)
		m_pd3dDepthStencilBuffer->Release();
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	m_nSwapChainBufferIndex = 0;
#else 
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, client_width, client_height,
		dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = 0;
#endif 
	CreateRenderTargetView();
	CreateDepthStencilView();
	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();
}

void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	m_pScene = new CScene();
	if (m_pScene)
		m_pScene->BuildObjects(pd3Device.Get(), m_pd3dCommandList);

	playerShader = new CPlayerShader();
	playerShader->CreateShader(pd3Device.Get(), m_pScene->GetGraphicsRootSignature());
	playerShader->BuildObjects(pd3Device.Get(), m_pd3dCommandList);

	m_pPlayer = (CPlayer*)playerShader->GetObjects()[0];

	m_pCamera = new CCamera();
	m_pCamera = m_pPlayer->ChangeCamera((DWORD)(0x03), m_GameTimer.GetTimeElapsed());
	m_pCamera->SetPlayer(m_pPlayer);

	//m_pCamera = m_pPlayer->ChangeCamera((DWORD)(0x03), m_GameTimer.GetTimeElapsed());

	/*m_pScene = new CScene();
	if (m_pScene)
		m_pScene->BuildObjects(pd3Device.Get(), m_pd3dCommandList);

	playerShader = new CPlayerShader();
	playerShader->CreateShader(pd3Device.Get(), m_pScene->GetGraphicsRootSignature());
	playerShader->BuildObjects(pd3Device.Get(), m_pd3dCommandList, m_pScene->GetTerrain());

	m_pPlayer = (CPlayer*)playerShader->GetObjects()[0];
	m_pCamera = new CCamera();
	m_pCamera = m_pPlayer->ChangeCamera((DWORD)(0x03),
		m_GameTimer.GetTimeElapsed());

	playerAct = playerStatus::Normal;*/
	//�� ��ü�� �����ϱ� ���Ͽ� �ʿ��� �׷��� ��� ����Ʈ���� ��� ť�� �߰��Ѵ�. 
	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//�׷��� ��� ����Ʈ���� ��� ����� ������ ��ٸ���. 
	WaitForGpuComplete();

	//�׷��� ���ҽ����� �����ϴ� ������ ������ ���ε� ���۵��� �Ҹ��Ų��. 
	if (m_pScene)
		m_pScene->ReleaseUploadBuffers();
	
	// �ʱ� ���콺 ��ġ�� ȭ�� �߾����� ��� ���콺Ŀ���� �����.
	m_ptOldCursorPos.x = FRAME_BUFFER_WIDTH / 2; m_ptOldCursorPos.y = FRAME_BUFFER_HEIGHT / 2;
	m_bMouseCapture = true;
	::ShowCursor(false);

	//if (playerShader)
	//	playerShader->ReleaseUploadBuffers();

	m_GameTimer.Reset();

}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) {
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();
	::CloseHandle(m_hFenceEvent);

#if defined(_DEBUG) 
	if (m_pd3dDebugController) m_pd3dDebugController->Release();
#endif
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (pdxgiSwapChain) pdxgiSwapChain->Release();

	//comptr�� ����
	//if (pd3Device) pd3Device->Release();
	//if (pdxgiFactory) pdxgiFactory->Release();


	if (m_hWnd) DestroyWindow(m_hWnd);
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	dwDirection = 0;

	if (::GetKeyboardState(pKeyBuffer))
	{
		if (pKeyBuffer[VK_W] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_S] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_A] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_D] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;
		
	}

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	
	/* ���콺 �Է¿� ���� �ൿ ��ȭ */
	
	
	//if (m_pSelectedObject)
	//{
	//	ProcessSelectedObject(dwDirection, cxDelta, cyDelta);
	//}
	
	if (m_bMouseCapture)
	{
		::GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		if (cxDelta || cyDelta) {
				m_pCamera->Rotate(cyDelta, cxDelta, 0.0f);
		}

		if (dwDirection && m_pPlayer->GetLive()) {
			m_pPlayer->Move(dwDirection, 100.0f * m_GameTimer.GetTimeElapsed(), false);
			//m_pPlayer->Move(dwDirection, 5.0f, false);
		}

	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float
	cyDelta)
{
	
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		//::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		
		break;
	case WM_MOUSEHOVER:
		
		break;
	case WM_MOUSELEAVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	BOOL fullScreenState = FALSE;
	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			m_bMouseCapture = !m_bMouseCapture;
			::ShowCursor(!m_bMouseCapture);
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			break;
		case VK_F8:
			break;
		case VK_F9:
			fullScreenState = FALSE;
			pdxgiSwapChain->GetFullscreenState(&fullScreenState, NULL);
			if (!fullScreenState) {
				DXGI_MODE_DESC dxgiTarget;
				dxgiTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				dxgiTarget.Width = client_width;
				dxgiTarget.Height = client_height;

				dxgiTarget.RefreshRate.Numerator = 60;
				dxgiTarget.RefreshRate.Denominator = 1;

				dxgiTarget.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				dxgiTarget.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				pdxgiSwapChain->ResizeTarget(&dxgiTarget);
			}
			pdxgiSwapChain->SetFullscreenState(!fullScreenState, NULL);

			OnResizeBackBuffers();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_ACTIVATE:
		
		break;
	case WM_SIZE:
		client_width = LOWORD(lParam);
		client_height = HIWORD(lParam);
		OnResizeBackBuffers();
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::AnimateObjects()
{
	if (m_pScene) {
		m_pScene->CrashObject(*m_pPlayer->getOOBB());
		m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
	}
	if (playerShader)
		m_pPlayer->Animate(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(60); //60������ ����

	AnimateObjects();

	ProcessInput();

	//AnimateObjects();

	//�÷��̾� ��ġ ����
	//printf("%f %f %f\n", m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, m_pPlayer->GetPosition().z);

	//d3d ǥ�� �κ�
	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);


	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//���� ���� Ÿ�ٿ� ���� ������Ʈ�� ���
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);


	//������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr +=
		(m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);
	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//���� Ÿ�� ��� ����-���ٽ� �並 ���-���� �ܰ�(OM)�� �����Ѵ�.
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle,
		TRUE, &d3dDsvCPUDescriptorHandle);


	//float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };/*Colors::Azure*/
	float pfClearColor[4] = { 1,1,1,1 };
	//���ϴ� �������� ���� Ÿ��(��)�� �����.
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor, 0, NULL);

	//���ϴ� ������ ����-���ٽ�(��)�� �����.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	

	////////////////////////////////////////////////
	//������
	if (m_pScene)
		m_pScene->Render(m_pd3dCommandList, m_pCamera);
	if (playerShader)
		playerShader->Render(m_pd3dCommandList, m_pCamera);


	////////////////////////////////////////////////

	//����Ÿ�ٿ� ���� ������ �����⸦ ���
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close(); //��� ����Ʈ�� ���� ���·� �����.

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	//��� ����Ʈ�� ��� ť�� �߰�
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);


	WaitForGpuComplete(); //GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.

						  //m_ui->DrawUI(m_hDCFrameBuffer, gameStatus::Gameloop, 0, 0, 0);
						  //PresentFrameBuffer();
						  //����ü�� ������Ʈ
	pdxgiSwapChain->Present(0, 0);



	MoveToNextFrame();



	//�����ӷ���Ʈ ǥ��
	_itow_s(m_GameTimer.GetFrameRate(), (m_pszFrameRate + 12), 37, 10);
	wcscat_s((m_pszFrameRate + 12), 37, _T(" FPS)"));
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}


void CGameFramework::WaitForGpuComplete() {
	//CPU �潺�� ���� �����Ѵ�. 
	const UINT64 nFence = ++m_nFenceValue[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFence); //GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�. 
	UINT64 T = m_pd3dFence->GetCompletedValue();
	if (m_pd3dFence->GetCompletedValue() < nFence) {
		//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
		hResult = m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame() {
	m_nSwapChainBufferIndex = pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValue[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

}

