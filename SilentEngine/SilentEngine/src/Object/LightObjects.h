#pragma once
#include "Shaders.h"




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
public:
	LightManagement();
	~LightManagement();

public:
	void BuildObject(ID3D12Device* pDevice, ID3D12GraphicsCommandList * pd3dCommandLis, float fAngle, XMFLOAT3& xmf3Axis);
	void UpdateShaderVariables();
	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera);
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


#ifdef _DEBUG_LIGHTOBJECT
class LightBoxShader : public ObjectShader
{
public:
	LightBoxShader();
	~LightBoxShader();

public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nRenderTargets = 1, void *pContext = NULL);
	virtual void SetMatrix(XMFLOAT4X4* mat);
};
#endif
