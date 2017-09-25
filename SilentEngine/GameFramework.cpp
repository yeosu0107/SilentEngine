#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	//게임 프레임워크 제어
	m_pScene = NULL;
	m_hInstance = NULL;
	m_hWnd = NULL;

	_tcscpy_s(m_pszFrameRate, _T("SileEngine ("));


	//d3d 제어
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

	//D3d 제어
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();



	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//오브젝트 제어

	BuildObjects();

	return(true);
}

//스왑체인
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
	//후면버퍼의 크기를 바탕화면 크기로 변경한다.
	dxgiSwapChainDesc.Flags = 0;
#else 
	// 바탕화면의 해상도를 후면버퍼의 크기에 맞게 변경한다. 
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	//스왑체인을 생성한다. 
	HRESULT hResult = pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc,
		(IDXGISwapChain **)&pdxgiSwapChain);
	//Alt + Enter 키의 동작을 비활성화
	pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//스왑체인의 현재 후면버퍼 인덱스를 저장한다.
	m_nSwapChainBufferIndex = pdxgiSwapChain->GetCurrentBackBufferIndex();
}

//d3d12 디바이스 생성
void CGameFramework::CreateDirect3DDevice() {
#if defined(_DEBUG) 
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&m_pd3dDebugController);
	m_pd3dDebugController->EnableDebugLayer();
#endif
	//DXGI 팩토리를 생성한다.
	//::CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void **)&pdxgiFactory);
	::CreateDXGIFactory1(IID_PPV_ARGS(&pdxgiFactory));
	IDXGIAdapter1 *pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++) {
		//어뎁터를 열거한다.
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		//어뎁터의 속성을 읽어온다.
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue; //플래그를 확인, 소프트웨어적으로 작동하면 다음 어뎁터로 넘어간다.
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pd3Device))))
			break; //플래그가 소프트웨어적으로 작동하지 않을 때 
				   //그 어뎁터를 이용 특성레벨 12.0을 지원하는 
				   //하드웨어 디바이스를 생성하고 성공여부를 확인
	}
	if (!pd3dAdapter) {
		//특성 레벨 12.0을 지원하는 하드웨어 디바이스를 
		//생성할 수 없으면 WARP 디바이스를 생성한다.
		pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&pd3Device);
	}
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링 
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	pd3Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다.
	nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다. 
	bMsaa4xEnable = (nMsaa4xQualityLevels > 1) ? true : false;

	//펜스를 생성하고 펜스 값을 1로 설정한다.
	pd3Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);


	if (pd3dAdapter)
		pd3dAdapter->Release();
}

//커맨드큐 생성
void CGameFramework::CreateCommandQueueAndList() {
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//직접(Direct) 명령 큐를 생성한다.
	HRESULT hResult = pd3Device->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);
	//직접(Direct) 명령 할당자를 생성한다.
	hResult = pd3Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator),
		(void **)&m_pd3dCommandAllocator);
	//직접(Direct) 명령 리스트를 생성한다.
	hResult = pd3Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator,
		NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close(); //명령 리스트는 생성되면 열린(Open) 상태이므로 닫힌(Closed) 상태로 만든다. 
}

//힙생성
void CGameFramework::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	//렌더 타겟 서술자 힙(서술자의 개수는 스왑체인 버퍼의 개수)을 생성한다. 
	HRESULT hResult = pd3Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	//렌더 타겟 서술자 힙의 원소의 크기를 저장한다.
	m_nRtvDescriptorIncrementSize = pd3Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//깊이-스텐실 서술자 힙(서술자의 개수는 1)을 생성한다. 
	hResult = pd3Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	//깊이-스텐실 서술자 힙의 원소의 크기를 저장한다. 
	m_nDsvDescriptorIncrementSize = pd3Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}


//랜더타겟뷰 생성
void CGameFramework::CreateRenderTargetView() {
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) {
		HRESULT hResult = pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dRenderTargetBuffers[i]);
		pd3Device->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

//뎁스스탠실뷰 생성
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

	//깊이-스텐실 버퍼를 생성한다.
	pd3Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//깊이-스텐실 버퍼 뷰를 생성한다. 
	pd3Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);

}

//백버퍼 리사이즈
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
	//씬 객체를 생성하기 위하여 필요한 그래픽 명령 리스트들을 명령 큐에 추가한다. 
	m_pd3dCommandList->Close();
	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다. 
	WaitForGpuComplete();

	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다. 
	if (m_pScene)
		m_pScene->ReleaseUploadBuffers();
	
	// 초기 마우스 위치를 화면 중앙으로 잡고 마우스커서를 감춘다.
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

	//comptr로 구현
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
	
	/* 마우스 입력에 따른 행동 변화 */
	
	
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
	m_GameTimer.Tick(60); //60프레임 고정

	AnimateObjects();

	ProcessInput();

	//AnimateObjects();

	//플레이어 위치 찍어보기
	//printf("%f %f %f\n", m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, m_pPlayer->GetPosition().z);

	//d3d 표현 부분
	//명령 할당자와 명령 리스트를 리셋한다.
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
	//현재 렌더 타겟에 대한 프리젠트가 대기
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);


	//현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들)를 계산한다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr +=
		(m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);
	//깊이-스텐실 서술자의 CPU 주소를 계산한다. 
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//렌더 타겟 뷰와 깊이-스텐실 뷰를 출력-병합 단계(OM)에 연결한다.
	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle,
		TRUE, &d3dDsvCPUDescriptorHandle);


	//float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };/*Colors::Azure*/
	float pfClearColor[4] = { 1,1,1,1 };
	//원하는 색상으로 렌더 타겟(뷰)을 지운다.
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor, 0, NULL);

	//원하는 값으로 깊이-스텐실(뷰)을 지운다.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	

	////////////////////////////////////////////////
	//랜더링
	if (m_pScene)
		m_pScene->Render(m_pd3dCommandList, m_pCamera);
	if (playerShader)
		playerShader->Render(m_pd3dCommandList, m_pCamera);


	////////////////////////////////////////////////

	//랜더타겟에 대한 랜더링 끝나기를 대기
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close(); //명령 리스트를 닫힌 상태로 만든다.

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	//명령 리스트를 명령 큐에 추가
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);


	WaitForGpuComplete(); //GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.

						  //m_ui->DrawUI(m_hDCFrameBuffer, gameStatus::Gameloop, 0, 0, 0);
						  //PresentFrameBuffer();
						  //스왑체인 프레젠트
	pdxgiSwapChain->Present(0, 0);



	MoveToNextFrame();



	//프레임레이트 표현
	_itow_s(m_GameTimer.GetFrameRate(), (m_pszFrameRate + 12), 37, 10);
	wcscat_s((m_pszFrameRate + 12), 37, _T(" FPS)"));
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}


void CGameFramework::WaitForGpuComplete() {
	//CPU 펜스의 값을 증가한다. 
	const UINT64 nFence = ++m_nFenceValue[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFence); //GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다. 
	UINT64 T = m_pd3dFence->GetCompletedValue();
	if (m_pd3dFence->GetCompletedValue() < nFence) {
		//펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다.
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

