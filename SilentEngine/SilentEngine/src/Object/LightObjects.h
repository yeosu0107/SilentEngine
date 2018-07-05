#pragma once
#include "Shaders.h"

#define LIGHT_MANAGER LightManagement::Instance()
#define MATERIAL_MANAGER  MaterialManagement::Instance()
//#define _DEBUG_LIGHTOBJECT


class ObjectShader;
class LightBoxShader;

class LightObject
{

public:
	LightObject();
	~LightObject();

	void SetLookAt(XMFLOAT3& xmf3Target);
	void RotateAxis(XMFLOAT3& xmf3RotateAxis, float fAngle);
	void CalculateLightMatrix();
	void SetPosition(XMFLOAT3& pos);
	void BuildObject(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pd3dCommandLis, LIGHT& light, float fangle, XMFLOAT3& xmf3RotateAxis, bool bShadowMap);
	VS_CB_CAMERA_INFO* LightMatrix();
	XMFLOAT4X4* WorldMatrix() { return &m_xmf4x4World; }

public:
	XMFLOAT3	Position() const { return m_Light.m_xmf3Position; };
	LIGHT		LightInfo() const { return m_Light; }
	
private:
	bool				m_bHadShadowMap;
	LIGHT				m_Light;

	XMFLOAT4X4			m_xmf4x4World;
	XMFLOAT4X4			m_xmf4x4ShadowMap;

	VS_CB_CAMERA_INFO	m_cbCameraInfo;

};

class LightManagement
{
public :
	static const unsigned int DIRECTIONLIGHT_START = 0;
	static const unsigned int DIRECTIONLIGHT_COUNT = 2;

	static const unsigned int SPOTLIGHT_START = 2;
	static const unsigned int SPOTLIGHT_COUNT = 4;

public:
	LightManagement();
	~LightManagement();

public:

	void BuildObject(ID3D12Device* pDevice, ID3D12GraphicsCommandList * pd3dCommandLis, float fAngle, XMFLOAT3& xmf3Axis);
	void UpdateShaderVariables();
	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
	void SetPosition(XMFLOAT3& pos, UINT index, XMFLOAT3& offset = XMFLOAT3());
	static LightManagement* Instance();


	VS_CB_CAMERA_INFO* LightMatrix(int index) { return  m_pLights[index].LightMatrix(); }
	LIGHT Light(int index) { return m_pLights[index].LightInfo(); }
	UploadBuffer<LIGHTS>* LightUploadBuffer() { return m_pd3dcbLights.get(); }

private:
	array<LightObject, MAX_LIGHTS>				m_pLights;
	unique_ptr<UploadBuffer<LIGHTS>>			m_pd3dcbLights = nullptr;

#ifdef _DEBUG_LIGHTOBJECT
	LightBoxShader* m_pShaderForTest;
#endif
};

class MaterialManagement
{
public:
	MaterialManagement();
	~MaterialManagement();

private:
	MATERIALS									m_pMat;
	unique_ptr<UploadBuffer<MATERIALS>>			m_pd3dcbMat;

public:
	void BuildObject(ID3D12Device* pDevice, ID3D12GraphicsCommandList * pd3dCommandLis);
	void UpdateShaderVariables();
	static MaterialManagement* Instance();
	//static MaterialManagement* Instance();
	UploadBuffer<MATERIALS>* MaterialUploadBuffer() { return m_pd3dcbMat.get(); }
};



#ifdef _DEBUG_LIGHTOBJECT
class LightBoxShader : public ObjectShader
{
public:
	LightBoxShader();
	~LightBoxShader();

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void SetMatrix(XMFLOAT4X4* mat);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList);
};
#endif
