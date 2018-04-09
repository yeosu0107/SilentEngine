#pragma once

#include "..\Model\ModelShader.h"

class PlayerShader : public DynamicModelShader
{
private:
	Camera * m_myCamera;
public:
	PlayerShader(int index, Camera* camera);
	~PlayerShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device * pd3dDevice);
	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets = 1, void * pContext = NULL);
	virtual void Animate(float fTimeElapsed);

	GameObject* getPlayer(int index) { return m_ppObjects[index]; }
};