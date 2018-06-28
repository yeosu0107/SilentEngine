#include "stdafx.h"
#include "D3DUtil.h"
#include "Framework.h"
#include "resource.h"
#include <windowsX.h>

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Framework::GetFramework()->MsgProc(hwnd, msg, wParam, lParam);
}

Framework* Framework::m_pFramework = nullptr;
Framework::Framework(HINSTANCE hInstance)
	: m_hFrameworkInst(hInstance)
{
	assert(m_pFramework == nullptr);
	m_pFramework = this;

	GlobalVal::getInstance()->LoadModels(new ModelLoader("GameModelList.csv"));
	GlobalVal::getInstance()->LoadMaps(new MapLoader("MapList.csv"));
	GlobalVal::getInstance()->LoadEffects(new EffectLoader("EffectList.csv"));
	GlobalVal::getInstance()->LoadFirePos(new FirePositionLoader("TochPosition.csv"));
}

Framework* Framework::GetFramework()
{
	return m_pFramework;
}

HINSTANCE Framework::FrameworkInst() const
{
	return m_hFrameworkInst;
}

HWND Framework::MainWnd() const
{
	return m_hMainWnd;
}

float Framework::AspectRatio() const
{
	return static_cast<float>(m_nClientWidth) / m_nClientHeight;
}

bool Framework::Get4xMsaaState() const
{
	return m_b4xMassState;
}

void Framework::Set4xMassState(bool value)
{
	if (m_b4xMassState != value)
	{
		m_b4xMassState = value;

		CreateSwapChain();
		OnResize();
	}
}

int Framework::Run()
{
	MSG msg = { 0 };

	m_Timer.Reset();
	
	while (msg.message != WM_QUIT) {
		
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
				CalculateFrameState();
				Update();
				RenderShadow();
				Render();
		}
	
	}
	return (int)msg.wParam;
}

bool Framework::Initialize()
{
	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	return true;
}
BOOL fullScreenState = FALSE;
LRESULT Framework::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			
			m_bMinimized = true;
			m_bMaximized = false;
		}
		else if (wParam == SIZE_MINIMIZED) {
			
			m_bMinimized = false;
			m_bMaximized = true;
		}
		else if (wParam == SIZE_RESTORED) {
			if (m_bMinimized) {
			
				m_bMinimized = false;
				//OnResize();
			}
			else if (m_bMaximized) {
	
				m_bMaximized = false;
				//OnResize();
			}
			else if (m_bResizing) {
			}

			else {
				//OnResize();
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		m_bResizing		= true;
		//m_Timer.Stop();
		return 0;

	case WM_EXITSIZEMOVE:
		
		m_bResizing = false;
		//m_Timer.Start();
		OnResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		OnMouseDown(wParam, msg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		OnMouseUp(wParam, msg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, msg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_KEYUP:
		//if (wParam == VK_ESCAPE) 
		//	PostQuitMessage(0);
		//else if ((int)wParam == VK_F2)
		//	Set4xMassState(!m_b4xMassState);

		if (wParam == VK_F9) {
			m_pSwapChain->GetFullscreenState(&fullScreenState, NULL);
			if (!fullScreenState) {
				DXGI_MODE_DESC dxgiTarget;
				dxgiTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				dxgiTarget.Width = m_nClientWidth;
				dxgiTarget.Height = m_nClientHeight;

				dxgiTarget.RefreshRate.Numerator = 60;
				dxgiTarget.RefreshRate.Denominator = 1;

				dxgiTarget.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				dxgiTarget.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				m_pSwapChain->ResizeTarget(&dxgiTarget);
			}
			m_pSwapChain->SetFullscreenState(!fullScreenState, NULL);
			m_pScene[m_nNowScene]->CaptureCursor();
			OnResize();
		}
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Framework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	::ZeroMemory(&rtvHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	rtvHeapDesc.NumDescriptors	= m_nSwapChainBuffers + NUM_RENDERTARGET;
	rtvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask		= 0;
	ThrowIfFailed(m_pD3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf()))
	);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors	= m_nDepthStencilBuffers;
	dsvHeapDesc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask		= 0;
	ThrowIfFailed(m_pD3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_pDsvHeap.GetAddressOf()))
	);

	m_nRtvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_nDsvIncresementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	::gnCbvSrvDescriptorIncrementSize = m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Framework::OnWakeUp()
{
	m_pScene = new Scene*[2];
	m_pDeferredFullScreenShader = make_unique<DeferredFullScreen>();
	m_GbufferDebug = make_unique<DrawGBuffers>();
	m_pDeferredFullScreenShader->SetNowScene(&m_nNowScene);
}

void Framework::OnResize()
{
	assert(m_pD3dDevice);
	assert(m_pSwapChain);
	assert(m_pDirectCmdListAlloc);

	FlushCommandQueue();

	/* 초기화 */
	ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));

	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_ppSwapChainBuffer[i].Reset();

	for ( int i = 0 ; i < m_nDepthStencilBuffers - NUM_DIRECTION_LIGHTS; ++i)
		m_pDepthStencilBuffer[i].Reset();

	ThrowIfFailed(m_pSwapChain->ResizeBuffers(
		m_nSwapChainBuffers,
		m_nClientWidth, m_nClientHeight,
		m_BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)
	);

	m_nCurrBuffer = 0;

	/* 생성 CreateSwapChainRenderTargetViews */

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	
	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pd3dRtvSwapChainBackBufferCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppSwapChainBuffer[i]);
		m_pD3dDevice->CreateRenderTargetView(m_ppSwapChainBuffer[i].Get(), &d3dRenderTargetViewDesc, m_pd3dRtvSwapChainBackBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}

	/* CreateDepthStencilView */

	DXGI_SAMPLE_DESC sampleDesc;
	::ZeroMemory(&sampleDesc, sizeof(DXGI_SAMPLE_DESC));
	sampleDesc.Count = m_b4xMassState ? 4 : 1;
	sampleDesc.Quality = m_b4xMassState ? (m_4xMsaaQuality - 1) : 0;

	D3D12_RESOURCE_DESC depthStencilDesc;
	::ZeroMemory(&depthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_nClientWidth;
	depthStencilDesc.Height = m_nClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc = sampleDesc;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	::ZeroMemory(&optClear, sizeof(D3D12_CLEAR_VALUE));
	optClear.Format = m_DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	// 쉐도우 맵을 위한 DSV 
	if (!m_bInitialized) {
		CTexture *pShadowMapTexture = new CTexture(NUM_DIRECTION_LIGHTS, RESOURCE_TEXTURE2D_SHADOWMAP, 0);
		for (UINT i = 1; i < m_nDepthStencilBuffers; ++i) {
			m_pDepthStencilBuffer[i] = pShadowMapTexture->CreateTexture(m_pD3dDevice.Get(), m_pCommandList.Get(),
				m_nClientWidth, m_nClientHeight,
				DXGI_FORMAT_R24G8_TYPELESS, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
				D3D12_RESOURCE_STATE_GENERIC_READ, &optClear, i - 1
			);
		}
		ShadowShader->BuildObjects(m_pD3dDevice.Get(), m_pCommandList.Get(), 1, pShadowMapTexture);
	}
	m_bInitialized = true;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	::ZeroMemory(&dsvDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	CTexture *pTexture = new CTexture(NUM_GBUFFERS, RESOURCE_TEXTURE2D, 0);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM,{ 1.0f, 1.0f, 1.0f, 1.0f } };
	for (UINT i = 0; i < NUM_RENDERTARGET; ++i)
	{
		m_ppd3dRenderTargetBuffers[i] = pTexture->CreateTexture(m_pD3dDevice.Get(), m_pCommandList.Get(), m_nClientWidth, m_nClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, i);
	}
	for (UINT i = 0; i < NUM_DEPTHGBUFFERS; ++i)
	{
		m_pDepthStencilBuffer[i] = pTexture->CreateTexture(m_pD3dDevice.Get(), m_pCommandList.Get(), m_nClientWidth, m_nClientHeight, 
			DXGI_FORMAT_R24G8_TYPELESS, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optClear, NUM_RENDERTARGET + i);
	}



	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle = DepthStencilView();
	for (int i = 0; i < m_nDepthStencilBuffers; ++i) {
		m_pD3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer[i].Get(), &dsvDesc, dsvCPUHandle);
		dsvCPUHandle.ptr += m_nDsvIncresementSize;
	}

	d3dRtvCPUDescriptorHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBuffers * m_nRtvDescriptorIncrementSize);


	for (UINT i = 0; i < NUM_RENDERTARGET; i++)
	{ 
		m_pd3dRtvRenderTargetBufferCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		m_pD3dDevice->CreateRenderTargetView(pTexture->GetTexture(i).Get(), &d3dRenderTargetViewDesc, m_pd3dRtvRenderTargetBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
	//pTexture->AddTexture(m_pDepthStencilBuffer[i].Get(), )

	m_pDeferredFullScreenShader->BuildObjects(m_pD3dDevice.Get(), m_pCommandList.Get(),1, pTexture);
	m_GbufferDebug->BuildObjects(m_pD3dDevice.Get(), m_pCommandList.Get(), 1, pTexture);

	ThrowIfFailed(m_pCommandList->Close());
	ID3D12CommandList* cmdList[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	FlushCommandQueue();

	m_ScreenViewport.TopLeftX			= 0;
	m_ScreenViewport.TopLeftY			= 0;
	m_ScreenViewport.Width				= static_cast<float>(m_nClientWidth);
	m_ScreenViewport.Height				= static_cast<float>(m_nClientHeight);
	m_ScreenViewport.MinDepth			= 0.0f;
	m_ScreenViewport.MaxDepth			= 1.0f;

	m_ScissorRect = { 0, 0, m_nClientWidth, m_nClientHeight };
}

void Framework::Update()
{
	m_Timer.Tick();

	OnKeyboardInput(m_Timer);

	if (m_pScene[m_nNowScene]->Update(m_Timer))
		m_bChangeScene = true;

	m_pDeferredFullScreenShader->Animate(m_Timer.DeltaTime());
}

void Framework::RenderShadow()
{
	auto pHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 1; i < m_nDepthStencilBuffers; ++i) {
		ThrowIfFailed(m_pDirectCmdListAlloc->Reset());
		ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));

		m_pCommandList->RSSetViewports(1, &m_ScreenViewport);
		m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer[i].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		pHandle.ptr += m_nDsvIncresementSize;

		m_pCommandList->ClearDepthStencilView(pHandle,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		m_pCommandList->OMSetRenderTargets(0, nullptr, false, &pHandle);
		m_pScene[m_nNowScene]->CreateShadowMap(m_pD3dDevice.Get(), m_pCommandList.Get(), i - 1);

		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer[i].Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

		ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
		ThrowIfFailed(m_pCommandList->Close());

		m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();
	}
}

//#define _SHADOWMAP_DEBUG
//#define _GRAPHICS_DEBUG

void Framework::Render()
{
	int i = 0;

	ThrowIfFailed(m_pDirectCmdListAlloc->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));

	m_pCommandList->RSSetViewports(1, &m_ScreenViewport);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	float pfClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

#ifdef _SHADOWMAP_DEBUG
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ClearRenderTargetView(CurrentBackBufferView(), pfClearColor, 0, nullptr);
	m_pCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ShadowShader->Render(m_pCommandList.Get(), m_pCamera);

	//m_pTestScene->Render(m_pD3dDevice.Get(), m_pCommandList.Get());

	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
#else
#ifdef _GRAPHICS_DEBUG

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ClearRenderTargetView(CurrentBackBufferView(), pfClearColor, 0, nullptr);
	m_pCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_pCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	m_pShadowShader->Render(m_pCommandList.Get(), m_pCamera);

	m_pTestScene->Render(m_pD3dDevice.Get(), m_pCommandList.Get());

	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };

#else

	/* 1차 렌더링 */
	
	for (i = 0; i < NUM_RENDERTARGET; ++i) {
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ppd3dRenderTargetBuffers[i].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));	
	}

	for (i = 0; i < NUM_DEPTHGBUFFERS; ++i) {
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer[i].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}

	for (int i = 0; i < NUM_RENDERTARGET; ++i) {
		m_pCommandList->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[i], pfClearColor, 0, NULL);
	}

	m_pCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_pCommandList->OMSetRenderTargets(NUM_RENDERTARGET, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &DepthStencilView());
	
	m_pScene[m_nNowScene]->Render(m_pD3dDevice.Get(), m_pCommandList.Get());

	ThrowIfFailed(m_pCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();

	ThrowIfFailed(m_pDirectCmdListAlloc->Reset());
	ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));

	/* 2차 렌더링 */
	for (i = 0; i < NUM_RENDERTARGET; ++i) {
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ppd3dRenderTargetBuffers[i].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	for (i = 0; i < NUM_DEPTHGBUFFERS; ++i) {
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer[i].Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//m_pCommandList->ClearDepthStencilView(m_pDsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_pCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nCurrBuffer], pfClearColor/*Colors::Azure*/, 0, NULL);
	m_pCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nCurrBuffer], TRUE, &DepthStencilView());


	m_pDeferredFullScreenShader->Render(m_pCommandList.Get(), m_pCamera);
	m_pScene[m_nNowScene]->RenderUI(m_pD3dDevice.Get(), m_pCommandList.Get());
	

	m_GbufferDebug->Render(m_pCommandList.Get(), m_pCamera);
#endif
#endif

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_pCommandList->Close());

	m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	ThrowIfFailed(m_pSwapChain->Present(0, 0));
	FlushCommandQueue();
	m_nCurrBuffer = (m_nCurrBuffer + 1) % m_nSwapChainBuffers;

	if (m_bChangeScene) {
		m_nNowScene = (m_nNowScene + 1) % 2;
		m_bChangeScene = false;
	}


}

bool Framework::InitMainWindow()
{
	DWORD flags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	WNDCLASS wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= MainWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= m_hFrameworkInst;
	wc.hIcon			= LoadIcon(0, IDI_APPLICATION);
	wc.hCursor			= LoadCursor(m_hFrameworkInst, MAKEINTRESOURCE(IDC_CURSOR1));
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, m_nClientWidth, m_nClientHeight };
	AdjustWindowRect(&R, flags, false);
	
	int nWidth = R.right - R.left;
	int nHeight = R.bottom - R.top;

	m_hMainWnd = CreateWindow(
		L"MainWnd", 
		m_sMainWndCaption.c_str(),
		flags,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		nWidth, 
		nHeight, 
		0, 
		0, 
		m_hFrameworkInst, 
		0
	);

	if (!m_hMainWnd) {
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWnd, SW_SHOW);
	UpdateWindow(m_hMainWnd);
	
	return true;
}

bool Framework::InitDirect3D()
{
	HRESULT hResult;


	ComPtr<ID3D12Debug> pDebugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)));
	pDebugController->EnableDebugLayer();


	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_pDxgiFactory)));

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) 
			continue;

		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pD3dDevice)))) 
			break;
	}

	if (!m_pD3dDevice)
	{
		hResult = m_pDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pd3dAdapter));
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pD3dDevice));
	}

	if (!m_pD3dDevice)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return false;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pD3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_4xMsaaQuality = d3dMsaaQualityLevels.NumQualityLevels;

	ThrowIfFailed(m_pD3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_pFence)));

	m_nRtvDescriptorSize				= m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_nDsvDescriptorSize				= m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_nCbvSrvUavDescriptorSize			= m_pD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	::ZeroMemory(&msQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	msQualityLevels.Format				= m_BackBufferFormat;
	msQualityLevels.SampleCount			= 4;
	msQualityLevels.Flags				= D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels	= 0;
	ThrowIfFailed(m_pD3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))
	);

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA qaulity level.");

#ifdef _DEBUG
	LogAdapters();
#endif
	OnWakeUp();
	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();
	OnResize();
	BuildObjects();

	return true;
}

void Framework::BuildObjects()
{
	m_ptOldCursorPos.x = (double)0.0; m_ptOldCursorPos.y = (double)0.0;

	m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr);

	GlobalVal::getInstance()->getModelLoader()->LodingModels(m_pD3dDevice.Get(), m_pCommandList.Get()); 
	GlobalVal::getInstance()->getMapLoader()->LodingModels(m_pD3dDevice.Get(), m_pCommandList.Get());
	
	LIGHT_MANAGER->BuildObject(m_pD3dDevice.Get(), m_pCommandList.Get(), 45.0f, XMFLOAT3(1.0f, 0.0f, 0.0f));
	MATERIAL_MANAGER->BuildObject(m_pD3dDevice.Get(), m_pCommandList.Get());

	MainScene* mainScene = new MainScene();
	mainScene->BuildScene(m_pD3dDevice.Get(), m_pCommandList.Get());
	m_pScene[0] = mainScene;

	TestScene* gameScene = new TestScene();
	m_pScene[1] = gameScene;
	gameScene->BuildScene(m_pD3dDevice.Get(), m_pCommandList.Get());
	m_pDeferredFullScreenShader->SetPlayer(gameScene->GetPlayer());
	m_pCamera = m_pScene[1]->GetCamera();

	ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
	m_pCommandList->Close();
	m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	FlushCommandQueue();
}

void Framework::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	::ZeroMemory(&commandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	commandQueueDesc.Type				= D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags				= D3D12_COMMAND_QUEUE_FLAG_NONE;
	
	ThrowIfFailed(m_pD3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_pCommandQueue.GetAddressOf())));

	ThrowIfFailed(m_pD3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_pDirectCmdListAlloc.GetAddressOf()))
	);
	
	ThrowIfFailed(m_pD3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pDirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_pCommandList.GetAddressOf()))
	);

	m_pCommandList->Close();
}

void Framework::CreateSwapChain()
{
	m_pSwapChain.Reset();

	DXGI_MODE_DESC modeDesc;
	::ZeroMemory(&modeDesc, sizeof(DXGI_MODE_DESC));
	modeDesc.Width						= m_nClientWidth;
	modeDesc.Height						= m_nClientHeight;
	modeDesc.RefreshRate.Numerator		= 60;
	modeDesc.RefreshRate.Denominator	= 1;
	modeDesc.Format						= m_BackBufferFormat;
	modeDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	modeDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	
	DXGI_SAMPLE_DESC sampleDesc;
	::ZeroMemory(&sampleDesc, sizeof(DXGI_SAMPLE_DESC));
	sampleDesc.Count = m_b4xMassState	? 4 : 1;
	sampleDesc.Quality = m_b4xMassState ? (m_4xMsaaQuality - 1) : 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	::ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= m_nSwapChainBuffers;
	swapChainDesc.OutputWindow			= m_hMainWnd;
	swapChainDesc.Windowed				= true;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SampleDesc			= sampleDesc;
	swapChainDesc.BufferDesc			= modeDesc;

	ThrowIfFailed(m_pDxgiFactory->CreateSwapChain(
			m_pCommandQueue.Get(),
			&swapChainDesc,
			m_pSwapChain.GetAddressOf())
		)
}

void Framework::FlushCommandQueue()
{
	m_nCurrentFence++;

	ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), m_nCurrentFence));
	
	if (m_pFence->GetCompletedValue() < m_nCurrentFence) {
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(m_pFence->SetEventOnCompletion(m_nCurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource * Framework::CurrentBackBuffer() const
{
	return m_ppSwapChainBuffer[m_nCurrBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Framework::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), 
		m_nCurrBuffer, 
		m_nRtvDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE Framework::DepthStencilView() const
{
	return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}


void Framework::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* pAdapter = nullptr;
	vector<IDXGIAdapter*> ppAdapterList;
	while (m_pDxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC desc;
		::ZeroMemory(&desc, sizeof(DXGI_ADAPTER_DESC));
		pAdapter->GetDesc(&desc);

		wstring sText = L"Adapter : ";
		sText += desc.Description;
		sText += L"\n";

		OutputDebugString(sText.c_str());

		ppAdapterList.push_back(pAdapter);

		++i;
	}

	for (size_t i = 0; i < ppAdapterList.size(); ++i) {
		LogAdapterOutputs(ppAdapterList[i]);
		ReleaseCom(ppAdapterList[i]);
	}
}

void Framework::LogAdapterOutputs(IDXGIAdapter* padapter)
{
	UINT i = 0;
	IDXGIOutput* pOutput = nullptr;

	while (padapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND) {
		DXGI_OUTPUT_DESC desc;
		::ZeroMemory(&desc, sizeof(DXGI_OUTPUT_DESC));
		pOutput->GetDesc(&desc);

		wstring sText = L"Output : ";
		sText += desc.DeviceName;
		sText += L"\n";
		OutputDebugString(sText.c_str());

		LogOutputDisplayModes(pOutput, m_BackBufferFormat);

		ReleaseCom(pOutput);

		++i;
	}
}

void Framework::LogOutputDisplayModes(IDXGIOutput* poutput, DXGI_FORMAT format)
{
	UINT nCount{ 0 };
	UINT nflags{ 0 };

	poutput->GetDisplayModeList(format, nflags, &nCount, nullptr);

	vector<DXGI_MODE_DESC> modeList(nCount);
	poutput->GetDisplayModeList(format, nflags, &nCount, &modeList[0]);

	for (auto& p : modeList) {
		UINT nNumerator = p.RefreshRate.Numerator;
		UINT nDeominator = p.RefreshRate.Denominator;
		wstring sText =
			L"Width = " + to_wstring(p.Width) + L" " +
			L"Height = " + to_wstring(p.Height) + L" " +
			L"Refresh = " + to_wstring(nNumerator) + L"/" + to_wstring(nDeominator) +
			L"\n";

		::OutputDebugString(sText.c_str());
	}
}

void Framework::CreateSwapChainRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pd3dRtvRenderTargetBufferCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		m_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppSwapChainBuffer[i]);
		m_pD3dDevice->CreateRenderTargetView(m_ppSwapChainBuffer[i].Get(), &d3dRenderTargetViewDesc, m_pd3dRtvRenderTargetBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void Framework::OnKeyboardInput(const Timer& gt)
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;

	if (GetKeyboardState(pKeysBuffer) && m_pScene[m_nNowScene]) {
		bProcessedByScene = m_pScene[m_nNowScene]->OnKeyboardInput(gt, m_hMainWnd);
	}
	if (bProcessedByScene)
	{
		m_bChangeScene = bProcessedByScene;
	}
}

void Framework::OnMouseDown(WPARAM btnState, UINT nMessageID, int x, int y)
{
	bool state = m_pScene[m_nNowScene]->OnMouseDown(m_hMainWnd, btnState, nMessageID, x, y);
	if (m_nNowScene == 0 && state == true) {
		reinterpret_cast<TestScene*>(m_pScene[1])->ResetScene(m_pD3dDevice.Get(), m_pCommandList.Get());
		/*m_pScene[1]->BuildScene(m_pD3dDevice.Get(), m_pCommandList.Get());
		m_pDeferredFullScreenShader->SetPlayer(reinterpret_cast<TestScene*>(m_pScene[1])->GetPlayer());
		m_pCamera = m_pScene[1]->GetCamera();*/
	}
}

void Framework::OnMouseUp(WPARAM btnState , UINT nMessageID, int x, int y)
{
	m_pScene[m_nNowScene]->OnMouseUp(m_hMainWnd, btnState, nMessageID, x, y);
}

void Framework::OnMouseMove(WPARAM btnState, UINT nMessageID, int x, int y)
{
	// 해당 씬의 마우스 캡쳐가 비활성화된 경우에만 마우스 좌표를 넘겨줌
	if(m_pScene[m_nNowScene] != nullptr && !m_pScene[m_nNowScene]->IsCaptureMouse())
		m_pScene[m_nNowScene]->OnMouseMove(static_cast<float>(x), static_cast<float>(y));
	if (GetCapture() == m_hMainWnd)
	{
		SetCursor(NULL);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Framework theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

void Framework::CalculateFrameState()
{
	static int nFrameCnt = 0;
	static float fTimeElapsed = 0.0f;

	nFrameCnt++;

	if ((m_Timer.TotalTime() - fTimeElapsed) >= 1.0f)
	{
		float fps = (float)nFrameCnt;
		float mspf = 1000.0f / fps;

		wstring sFpsStr = to_wstring(fps);
		wstring sMspfStr = to_wstring(mspf);

		wstring sWindowText = m_sMainWndCaption +
			L"	fps : " + sFpsStr +
			L"	mspf : " + sMspfStr;

		SetWindowText(m_hMainWnd, sWindowText.c_str());

		nFrameCnt = 0;
		fTimeElapsed += 1.0f;
	}
}