#pragma once

//-----------------------------------------------------------------------------
// File: GameFramework.h
//-----------------------------------------------------------------------------

#ifndef _GAME_APPLICATION_FRAMEWORK_H_
#define _GAME_APPLICATION_FRAMEWORK_H_

#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "Texture.h"

class CGameFramework
{
private:
	CScene						*m_pScene;
	HINSTANCE					m_hInstance;
	HWND						m_hWnd;

	int							client_width;
	int							client_height;

	//D3D 변수
	//팩토리 인터페이스 포인터
	ComPtr<IDXGIFactory4>		pdxgiFactory;
	//스왑체인 인터페이스 포인터
	IDXGISwapChain3*			pdxgiSwapChain;
	//D3D12 디바이스 인터페이스 포인터
	ComPtr<ID3D12Device>		pd3Device;
	//다중샘플링 활성/비활성
	bool						bMsaa4xEnable = false;
	//다중샘플링 퀄리티 레벨 지정
	UINT						nMsaa4xQualityLevels = 0;
	//스왑 체인의 후면 버퍼의 갯수
	static const UINT m_nSwapChainBuffers = 2;
	//현재 스왑 체인의 후면 버퍼 인덱스
	UINT m_nSwapChainBufferIndex;

	//랜더타겟버퍼
	ID3D12Resource *m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	//서술자 십 인터페이스 포인터
	ID3D12DescriptorHeap *m_pd3dRtvDescriptorHeap;
	//랜더타겟 서술자의 버퍼 크기
	UINT m_nRtvDescriptorIncrementSize;

	//깊이-스텐실 버퍼
	ID3D12Resource *m_pd3dDepthStencilBuffer;
	//서술자 힙 인터페이스 포인터
	ID3D12DescriptorHeap *m_pd3dDsvDescriptorHeap;
	//깊이-스탠실 서술자 원소 크기
	UINT m_nDsvDescriptorIncrementSize;

	//명령 큐
	ID3D12CommandQueue *m_pd3dCommandQueue;
	//명령 어로케이터
	ID3D12CommandAllocator *m_pd3dCommandAllocator;
	//명령리스트
	ID3D12GraphicsCommandList *m_pd3dCommandList;

	//펜스 인터페이스 포인터
	ID3D12Fence *m_pd3dFence;
	//펜스 값
	UINT64 m_nFenceValue[m_nSwapChainBuffers];
	//펜스 핸들
	HANDLE m_hFenceEvent;

#if defined(_DEBUG) 
	ID3D12Debug *m_pd3dDebugController;
#endif
	
	CGameTimer					m_GameTimer;
	_TCHAR						m_pszFrameRate[50];

	bool						CameraCorrection;

public:
	CPlayerShader* playerShader = nullptr;

	CPlayer* m_pPlayer = nullptr;
	CCamera *m_pCamera = nullptr;
	POINT m_ptOldCursorPos;
	DWORD dwDirection = 0;

	//TRACKMOUSEEVENT mouseEvent;

	CGameFramework(void);
	~CGameFramework(void);

	//게임 프레임워크 제어
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	void FrameAdvance();


	//D3D제어
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateCommandQueueAndList();

	void CreateRenderTargetView();
	void CreateDepthStencilView();

	void OnResizeBackBuffers();

	void WaitForGpuComplete();

	void MoveToNextFrame();

	//마우스, 키보드 입출력
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);

	//오브젝트 제어
	void BuildObjects();
	void ReleaseObjects();
	void AnimateObjects();
	void ProcessInput();


};



#endif 