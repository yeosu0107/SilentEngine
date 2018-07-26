#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "D3DUtil.h"
#include "Timer.h"
#include "Scene.h"
//#include "..\Model\ModelLoader.h"

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
	virtual void CreateSwapChainRenderTargetViews(D3D12_CPU_DESCRIPTOR_HANDLE& descHandle);

	virtual void OnWakeUp();
	virtual void OnResize();
	
	virtual void Update();
	virtual void RenderShadow();
	virtual void RenderOBJ();
	virtual void RenderDeffered();
	virtual void RenderOutlineFog();
	virtual void DispatchComputeShaders();
	virtual void RenderHDR();
	virtual void RenderHPBars();
	virtual void Render();

	virtual void OnKeyboardInput(const Timer& gt);
	virtual void OnMouseDown(WPARAM btnState, UINT nMessageID, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, UINT nMessageID, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, UINT nMessageID, int x, int y);

protected:

	bool InitMainWindow();
	bool InitDirect3D();
	void BuildObjects();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateShadowMap();
	void CreateDepthStencilViews();
	void CreateComputeOutputBuffers(CTexture* pTexture);
	void CreateRenderTargetViews(D3D12_CPU_DESCRIPTOR_HANDLE& descHandle, CTexture* pTexture, DXGI_FORMAT format, UINT start, UINT count);
	void CreateRenderTarget(CTexture* pTexture);
	void CreateHDRRenderTarget(CTexture* pTexture);

	void ExcuteCommandList();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void CalculateFrameState();

	void ClearRTVnDSV();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* padapter);
	void LogOutputDisplayModes(IDXGIOutput* poutput, DXGI_FORMAT format);
protected:

	static Framework* m_pFramework;

	HINSTANCE m_hFrameworkInst = nullptr;
	HWND m_hMainWnd = nullptr;
	bool m_bMinimized = false;
	bool m_bMaximized = false;
	bool m_bResizing = false;
	bool m_bFullscreenState = false;
	bool m_bInitialized = false;
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

	static const int m_nSwapChainBuffers = 2;
	int m_nCurrBuffer = 0;
	ComPtr<ID3D12Resource> m_ppSwapChainBuffer[m_nSwapChainBuffers];

	static const int m_nDepthStencilBuffers = 1 + NUM_DIRECTION_LIGHTS;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer[m_nDepthStencilBuffers];
	UINT					m_nDsvIncresementSize;

	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	UINT m_nRtvDescriptorSize = 0;
	UINT m_nDsvDescriptorSize = 0;
	UINT m_nCbvSrvUavDescriptorSize = 0;

	wstring m_sMainWndCaption = L"SiN ";
	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

	//static const UINT					m_nRenderTargetBuffers = 2;
	ComPtr<ID3D12Resource>				m_ppd3dRenderTargetBuffers[NUM_RENDERTARGET];
	ComPtr<ID3D12Resource>				m_ppd3dHDRBuffers[NUM_HDRBUFFER];
	ComPtr<ID3D12Resource>				m_ppd3dLightMapBuffers[NUM_LIGHTMAP];

	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dRtvRenderTargetBufferCPUHandles[NUM_RENDERTARGET + NUM_HDRBUFFER];
	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dRtvLightMapBufferCPUHandle[NUM_LIGHTMAP];
	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBuffers];
	UINT								m_nRtvDescriptorIncrementSize;

	DXGI_FORMAT							m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT							m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int									m_nClientWidth = FRAME_BUFFER_WIDTH;
	int									m_nClientHeight = FRAME_BUFFER_HEIGHT;

	RECT									m_clientRect;
	POINT								pt1, pt2;

	const UINT							m_nMaxScene = 3;
	UINT								m_nNowScene = 0;

protected:
	Scene**								m_pScene;
	POINT								m_ptOldCursorPos;

	bool								m_bDebugGBuffers = false;
	bool								m_bMouseCapture = false;
	bool								m_bChangeScene = false;
	float								m_fMouseSensitive = 4.5f;	// 마우스 민감도

	Camera*								m_pCamera = nullptr;

	unique_ptr<DeferredFullScreen>		m_pDeferredFullScreenShader;
	unique_ptr<HDRShader>				m_HDRShader = nullptr;

	unique_ptr<DrawGBuffers>			m_GbufferDebug;
	unique_ptr<OutlineFogShader>		m_OutlineShader;
};

