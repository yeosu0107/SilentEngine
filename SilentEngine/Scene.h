#pragma once
#include "Shader.h"
#include "Camera.h"


class CScene
{
private:

protected:
	//씬은 셰이더들의 집합이다. 셰이더들은 게임 객체들의 집합이다.
	//랜더를 위해 사용
	CShader** m_ppShaders = nullptr;
	int m_nShaders = 0;

	


	ID3D12RootSignature* m_pd3dGraphicsRootSignature = nullptr;


public:
	CScene();
	~CScene();

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ReleaseUploadBuffers();


	//루트 시그너쳐 생성
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature();

	

	//파이프라인 상태를 나타내는 인터페이스 포인터 
	//ID3D12PipelineState *m_pd3dPipelineState; 
};

