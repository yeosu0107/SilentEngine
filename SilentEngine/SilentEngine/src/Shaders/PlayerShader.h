#pragma once

#include "..\Model\ModelShader.h"

class PlayerShader : public DynamicModelShader
{
private:
	Camera * m_myCamera;
public:
	PlayerShader(int index, Camera* camera);
	~PlayerShader();

	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext);
	virtual void Animate(float fTimeElapsed);

	GameObject* getPlayer(int index) { return m_ppObjects[index]; }
};