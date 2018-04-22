#include "stdafx.h"
#include "LightObjects.h"
#include "D3DUtil.h"


LightObject::LightObject()
{
}

void LightObject::BuildObject(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pd3dCommandLis, LIGHT& light, float fAngle, XMFLOAT3 & xmf3RotateAxis, bool bShadowMap)
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4ShadowMap = Matrix4x4::Identity();
	m_Light = light;
	m_bHadShadowMap = bShadowMap;
	SetLookAt(m_Light.m_xmf3Direction);
	RotateAxis(xmf3RotateAxis, fAngle);

	if (bShadowMap)
		CalculateLightMatrix();
}

VS_CB_CAMERA_INFO* LightObject::LightMatrix()
{
	if(m_bHadShadowMap)
		return &m_cbCameraInfo;

	return nullptr;
}

LightObject::~LightObject()
{
}

void LightObject::SetLookAt(XMFLOAT3 & xmf3Target)
{
	XMFLOAT3 xmf3Up(0.0f, 0.0f, 1.0f);
	XMFLOAT3 xmf3Position = m_Light.m_xmf3Position;
	XMFLOAT3 xmf3Look = xmf3Target;
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	m_xmf4x4World._11 = xmf3Right.x; m_xmf4x4World._12 = xmf3Right.y; m_xmf4x4World._13 = xmf3Right.z;
	m_xmf4x4World._21 = xmf3Up.x; m_xmf4x4World._22 = xmf3Up.y; m_xmf4x4World._23 = xmf3Up.z;
	m_xmf4x4World._31 = xmf3Look.x; m_xmf4x4World._32 = xmf3Look.y; m_xmf4x4World._33 = xmf3Look.z;
	m_xmf4x4World._41 = xmf3Position.x; m_xmf4x4World._42 = xmf3Position.y; m_xmf4x4World._43 = xmf3Position.z;
}

void LightObject::RotateAxis(XMFLOAT3 & xmf3RotateAxis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotateAxis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	m_Light.m_xmf3Direction = XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33);
}

void LightObject::CalculateLightMatrix()
{
	XMFLOAT3	lightDir = XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33);
	XMFLOAT3	lightPos = XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT3	lightUp = XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23);

	XMFLOAT4X4	lightView = Matrix4x4::LookToLH(lightPos, lightDir, lightUp);
	
	float l =  -250.0f; 
	float b =  -500.0f;
	float n =  +0.001f;
	float r =  +250.0f;
	float t =  +500.0f;
	float f =  +1000.0f; 

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = XMLoadFloat4x4(&lightView) * lightProj * T;

	XMStoreFloat4x4(&m_cbCameraInfo.m_xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&lightView)));
	XMStoreFloat4x4(&m_cbCameraInfo.m_xmf4x4Projection, XMMatrixTranspose(lightProj));
	XMStoreFloat4x4(&m_cbCameraInfo.m_xmf4x4ShadowProjection[0], XMMatrixTranspose(S));
	::memcpy(&m_cbCameraInfo.m_xmf3Position, &lightPos, sizeof(XMFLOAT3));
}


LightManagement::LightManagement()
{
}


LightManagement::~LightManagement()
{
}

void LightManagement::BuildObject(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pd3dCommandLis, float fAngle, XMFLOAT3& xmf3Axis)
{
	m_pd3dcbLights = std::make_unique<UploadBuffer<LIGHTS>>(pDevice, 1, true);

	LIGHTS lights;
	::ZeroMemory(&lights, sizeof(LIGHTS));

	lights.m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	lights.m_pLights[0].m_bEnable = true;
	lights.m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	lights.m_pLights[0].m_fRange = 300.0f;
	lights.m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	lights.m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights.m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	lights.m_pLights[0].m_xmf3Position = XMFLOAT3(-0.0f, 100.f, 430.0f);
	lights.m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//lights.m_pLights[0].m_xmf3Attenuation = XMFLOAT3(0.1f, 0.01f, 0.0001f);
	//lights.m_pLights[0].m_fFalloff = 40.0f;
	//lights.m_pLights[0].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	//lights.m_pLights[0].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	lights.m_pLights[1].m_bEnable = false;
	lights.m_pLights[1].m_nType = DIRECTIONAL_LIGHT;
	lights.m_pLights[1].m_fRange = 300.0f;
	lights.m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights.m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights.m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	lights.m_pLights[1].m_xmf3Position = XMFLOAT3(200.f, 0.0f, 28.442f);
	lights.m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	lights.m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//lights.m_pLights[1].m_fFalloff = 40.0f;
	//lights.m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	//lights.m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	lights.m_pLights[2].m_bEnable = true;
	lights.m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	lights.m_pLights[2].m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lights.m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	lights.m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	lights.m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);

	lights.m_pLights[3].m_bEnable = false;
	lights.m_pLights[3].m_nType = SPOT_LIGHT;
	lights.m_pLights[3].m_fRange = 60.0f;
	lights.m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.1f, 1.0f, 1.0f, 1.0f);
	lights.m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	lights.m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	lights.m_pLights[3].m_xmf3Position = XMFLOAT3(-150.0f, 30.0f, 30.0f);
	lights.m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	lights.m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	lights.m_pLights[3].m_fFalloff = 8.0f;
	lights.m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	lights.m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	for (int i = 0; i < MAX_LIGHTS; ++i) {
		LIGHT l = lights.m_pLights[i];
		m_pLights[i].BuildObject(pDevice, pd3dCommandLis, l, fAngle, xmf3Axis, (l.m_nType == DIRECTIONAL_LIGHT));
	}

	m_pd3dcbLights->CopyData(0, lights);

#ifdef _DEBUG_LIGHTOBJECT
	vector<XMFLOAT4X4> data(MAX_LIGHTS);

	for (int i = 0; i < MAX_LIGHTS; ++i) {
		data[i] = *(m_pLights[i].WorldMatrix());
	}
	m_pShaderForTest = new LightBoxShader();
	m_pShaderForTest->BuildObjects(pDevice, pd3dCommandLis, 2, &lights);
	m_pShaderForTest->SetMatrix(data.data());

#endif
}

void LightManagement::UpdateShaderVariables()
{
	LIGHTS lights;
	lights.m_xmf4GlobalAmbient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < MAX_LIGHTS; ++i) {
		lights.m_pLights[i] = m_pLights[i].LightInfo();
	}

	m_pd3dcbLights->CopyData(0, lights);
}

void LightManagement::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
#ifdef _DEBUG_LIGHTOBJECT
	if (m_pShaderForTest)
		m_pShaderForTest->Render(pd3dCommandList, pCamera);
#endif
}


#ifdef _DEBUG_LIGHTOBJECT
LightBoxShader::LightBoxShader()
{
}

LightBoxShader::~LightBoxShader()
{
}

void LightBoxShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	LIGHTS lights = *(LIGHTS*)pContext;
	m_nObjects = MAX_LIGHTS - 2;
	m_nPSO = 1;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "VSTextured", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\color.hlsl", nullptr, "PSTextured", "ps_5_0");

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2DARRAY, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\bricks.dds", 0);

	UINT ncbElementBytes = D3DUtil::CalcConstantBufferByteSize(sizeof(CB_GAMEOBJECT_INFO));

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_ObjectCB->Resource(), ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	MeshGeometryTextured *pCubeMesh = new MeshGeometryTextured(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 20.0f);

	m_ppObjects = vector<GameObject*>(m_nObjects);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i] = new GameObject();
		m_ppObjects[i]->SetPosition(lights.m_pLights[i].m_xmf3Position);
		m_ppObjects[i]->SetMesh(0, pCubeMesh);
		m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	}
}

void LightBoxShader::SetMatrix(XMFLOAT4X4 * mat)
{
	for (unsigned int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i]->SetMatrix(mat[i]);
	}
}

#endif