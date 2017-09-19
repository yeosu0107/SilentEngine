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

	//D3D ����
	//���丮 �������̽� ������
	ComPtr<IDXGIFactory4>		pdxgiFactory;
	//����ü�� �������̽� ������
	IDXGISwapChain3*			pdxgiSwapChain;
	//D3D12 ����̽� �������̽� ������
	ComPtr<ID3D12Device>		pd3Device;
	//���߻��ø� Ȱ��/��Ȱ��
	bool						bMsaa4xEnable = false;
	//���߻��ø� ����Ƽ ���� ����
	UINT						nMsaa4xQualityLevels = 0;
	//���� ü���� �ĸ� ������ ����
	static const UINT m_nSwapChainBuffers = 2;
	//���� ���� ü���� �ĸ� ���� �ε���
	UINT m_nSwapChainBufferIndex;

	//����Ÿ�ٹ���
	ID3D12Resource *m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	//������ �� �������̽� ������
	ID3D12DescriptorHeap *m_pd3dRtvDescriptorHeap;
	//����Ÿ�� �������� ���� ũ��
	UINT m_nRtvDescriptorIncrementSize;

	//����-���ٽ� ����
	ID3D12Resource *m_pd3dDepthStencilBuffer;
	//������ �� �������̽� ������
	ID3D12DescriptorHeap *m_pd3dDsvDescriptorHeap;
	//����-���Ľ� ������ ���� ũ��
	UINT m_nDsvDescriptorIncrementSize;

	//��� ť
	ID3D12CommandQueue *m_pd3dCommandQueue;
	//��� ���������
	ID3D12CommandAllocator *m_pd3dCommandAllocator;
	//��ɸ���Ʈ
	ID3D12GraphicsCommandList *m_pd3dCommandList;

	//�潺 �������̽� ������
	ID3D12Fence *m_pd3dFence;
	//�潺 ��
	UINT64 m_nFenceValue[m_nSwapChainBuffers];
	//�潺 �ڵ�
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

	//���� �����ӿ�ũ ����
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();
	void FrameAdvance();


	//D3D����
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateCommandQueueAndList();

	void CreateRenderTargetView();
	void CreateDepthStencilView();

	void OnResizeBackBuffers();

	void WaitForGpuComplete();

	void MoveToNextFrame();

	//���콺, Ű���� �����
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);

	//������Ʈ ����
	void BuildObjects();
	void ReleaseObjects();
	void AnimateObjects();
	void ProcessInput();


};



#endif 