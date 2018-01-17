#pragma once

#define FRAME_BUFFER_WIDTH		1280
#define FRAME_BUFFER_HEIGHT		720

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include <memory>

using namespace std;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	// 게임 프레임워크 제어
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	// D3D제어
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChainRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();
	void CreateRenderTargetViews();

	void OnResizeBackBuffers();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	// 마우스 키보드 제어
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	// 오브젝트 제어
	void BuildObjects();
	void ReleaseObjects();
	void ProcessInput();
	void AnimateObjects();

private:
	HINSTANCE							m_hInstance;
	HWND								m_hWnd; 

	int									m_nWndClientWidth;
	int									m_nWndClientHeight;
        
	ComPtr<IDXGIFactory4>				m_pdxgiFactory;
	ComPtr<IDXGISwapChain3>				m_pdxgiSwapChain;
	ComPtr<ID3D12Device>				m_pd3dDevice;

	bool								m_bMsaa4xEnable = false;
	UINT								m_nMsaa4xQualityLevels = 0;

	static const UINT					m_nSwapChainBuffers = 2;
	UINT								m_nSwapChainBufferIndex;

	ComPtr<ID3D12Resource>				m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap>		m_pd3dRtvDescriptorHeap;
	UINT								m_nRtvDescriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBuffers];

	ComPtr<ID3D12Resource>				m_pd3dDepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_pd3dDsvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dDsvDepthStencilBufferCPUHandle;

	ComPtr<ID3D12CommandAllocator>		m_pd3dCommandAllocator;
	ComPtr<ID3D12CommandQueue>			m_pd3dCommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_pd3dCommandList;

	ComPtr<ID3D12Fence>					m_pd3dFence;
	UINT64								m_nFenceValues[m_nSwapChainBuffers];
	HANDLE								m_hFenceEvent = NULL;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug>					m_pd3dDebugController;
#endif

	CGameTimer							m_GameTimer;

	CScene**							m_ppScene;
	CCamera*							m_pCamera;
	CPlayer*							m_pPlayer;

	static const UINT					m_nRenderTargetBuffers = 2;	// 랜더 타겟 버퍼( 2개인 이유 : 1개는 일단 그리고 엣지를 찾는 용, 1개는 완성 본 )
	ComPtr<ID3D12Resource>				m_ppd3dRenderTargetBuffers[m_nRenderTargetBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dRtvRenderTargetBufferCPUHandles[m_nRenderTargetBuffers];

//	D3D12_CPU_DESCRIPTOR_HANDLE			m_pd3dSrvCPUHandles[m_nRenderTargetBuffers];
//	D3D12_GPU_DESCRIPTOR_HANDLE			m_pd3dSrvGPUHandles[m_nRenderTargetBuffers];

	CTextureToFullScreenByLaplacianShader*	 m_pTextureToFullScreenByLaplacianShader;	 // Laplacian이라는 엣지를 찾는 방법을 이용해서 화면전체에 출력하는 셰이더
	
	POINT							m_ptOldCursorPos;

	_TCHAR							m_pszFrameRate[50];

	int								m_nMaxScene;
	int								m_nNowScene;
	bool							m_bEndScene = false;

// 카메라 관련 변수
	bool							CameraCorrection;
	bool							m_bMouseCapture;
	float							m_fMouseSensitive;	// 마우스 민감도
};

