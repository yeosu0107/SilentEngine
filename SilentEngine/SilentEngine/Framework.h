#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "D3DUtil.h"
#include "Timer.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class Framework
{
public:

	Framework(HINSTANCE hInstance);
	Framework(const Framework& rhs) {};
	Framework& operator=(const Framework& rhs) {};
	virtual ~Framework() {};

public:
	
	static Framework* GetFramework();

	HINSTANCE FrameworkInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	bool Get4xMsaaState() const;
	void Set4xMassState(bool value);

	int Run();

	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:

	virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();
	virtual void Update(const Timer& gt);
	virtual void Render(const Timer& gt);

	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:

	bool InitMainWindow();
	bool InitDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void CalculateFrameState();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* padapter);
	void LogOutputDisplayModes(IDXGIOutput* poutput, DXGI_FORMAT format);

protected:

	static Framework* m_pFramework;

	HINSTANCE m_hFrameworkInst = nullptr;
	HWND m_hMainWnd = nullptr;
	bool m_bAppPaused = false;
	bool m_bMinimized = false;
	bool m_bMaximized = false;
	bool m_bResizing = false;
	bool m_bFullscreenState = false;

	bool m_b4xMassState = false;
	UINT m_4xMsaaQuality = 0;

	Timer m_Timer;

	ComPtr<IDXGIFactory4> m_pDxgiFactory;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<ID3D12Device> m_pD3dDevice;

	ComPtr<ID3D12Fence> m_pFence;
	UINT64 m_nCurrentFence = 0;

	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	ComPtr<ID3D12CommandAllocator> m_pDirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

	static const int SwapChainBufferCount = 2;
	int m_nCurrBuffer = 0;
	ComPtr<ID3D12Resource> m_ppSwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	UINT m_nRtvDescriptorSize = 0;
	UINT m_nDsvDescriptorSize = 0;
	UINT m_nCbvSrvUavDescriptorSize = 0;

	wstring m_sMainWndCaption = L"SiN ";
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	int m_nClientWidth = 800;
	int m_nClientHeight = 600;

};

