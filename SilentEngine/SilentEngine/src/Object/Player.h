#pragma once

#include "D3DUtil.h"
#include "..\Model\ModelObject.h"
//#include "..\Model\ModelShader.h"

#define ASPECT_RATIO (float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

#define FIRST_PERSON_CAMERA 0x01
#define THIRD_PERSON_CAMERA 0x03

#define ROOT_PARAMETER_CAMERA		0
#define ROOT_PARAMETER_OBJECT		1
#define ROOT_PARAMETER_PLAYER		1
#define ROOT_PARAMETER_MATERIAL		2
#define ROOT_PARAMETER_LIGHT		3
#define ROOT_PARAMETER_TEXTURE		4

class Player : public ModelObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	Camera* m_pCamera;
public:
	Player(LoadModel* model, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	~Player();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void Move(DWORD dir, float fDist);
	virtual void Animate(float fTime);
};
//class MyPlayer : public Player
//{
//public:
//	
//};