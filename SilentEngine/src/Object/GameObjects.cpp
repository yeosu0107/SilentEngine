#include "stdafx.h"
#include "GameObjects.h"
#include "Shaders.h"


CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextures = nTextures;
	m_nTexType = nTextureType;
	m_nSamplers = nSamplers;

	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = vector<SRVROOTARGUMENTINFO>();
		m_ppd3dTextureUploadBuffers = vector<ComPtr<ID3D12Resource>>(m_nTextures);
		m_ppd3dTextures = vector<ComPtr<ID3D12Resource>>(m_nTextures);
		m_pTextureType = vector<UINT>(m_nTextures);
		for (int i = 0; i < m_nTextures; i++) {
			m_ppd3dTextureUploadBuffers[i] = m_ppd3dTextures[i] = nullptr;
			m_pTextureType[i] = m_nTexType;
		}
	}

	if (m_nSamplers > 0) 
		m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (!m_ppd3dTextures.empty())
	{
		m_ppd3dTextures.clear();
	}

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	SRVROOTARGUMENTINFO info;
	info.m_nRootParameterIndex = nRootParameterIndex;
	info.m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
	m_pRootArgumentInfos.push_back(info);
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::AddTexture(ID3D12Resource * texture, ID3D12Resource * uploadbuffer, UINT textureType)
{
	m_nTextures++;
	m_ppd3dTextures.push_back(texture);
	m_ppd3dTextureUploadBuffers.push_back(uploadbuffer);
	m_pTextureType.push_back(textureType);
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTextureType[0] == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			// 바인딩할 슬롯 넘버
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateComputeShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	if (m_pTextureType[0] == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetComputeRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			// 바인딩할 슬롯 넘버
			pd3dCommandList->SetComputeRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (!m_ppd3dTextureUploadBuffers.empty())
	{
		m_ppd3dTextureUploadBuffers.clear();
	}
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = D3DUtil::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, m_ppd3dTextureUploadBuffers[nIndex].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

ComPtr<ID3D12Resource> CTexture::CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = D3DUtil::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	if (dxgiFormat == DXGI_FORMAT_R24G8_TYPELESS) m_pTextureType[nIndex] = RESOURCE_TEXTURE2D_SHADOWMAP;

	return(m_ppd3dTextures[nIndex]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pShader) m_pShader->Release();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	if (m_pTexture) 
		m_pTexture.release();

	m_pTexture = make_unique<CTexture>(std::move(*pTexture));
}

void CMaterial::SetShader(Shaders *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTexture) m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) m_pShader->ReleaseShaderVariables();
	if (m_pTexture) m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
GameObject::GameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = vector<unique_ptr<MeshGeometry>>(m_nMeshes);
		for (UINT i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = nullptr;
	}

	m_moveSpeed = 0.0f;
}

GameObject::~GameObject()
{
	ReleaseShaderVariables();

	if (!m_ppMeshes.empty())
	{
	}
	if (m_pMaterial) m_pMaterial->Release();
}

void GameObject::SetMesh(int nIndex, MeshGeometry *pMesh)
{
	if (!m_ppMeshes.empty())
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex].release();
		m_ppMeshes[nIndex] = make_unique<MeshGeometry>(*pMesh);
	}
}



void GameObject::SetShader(Shaders *pShader)
{
	if (!m_pMaterial)
	{
		CMaterial *pMaterial = new CMaterial();
		SetMaterial(pMaterial);
	}
	if (m_pMaterial) m_pMaterial->SetShader(pShader);
}

void GameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void GameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void GameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->~UploadBuffer();
	}
	if (m_pMaterial) m_pMaterial->ReleaseShaderVariables();
}

void GameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	//XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	//if (m_pMaterial) 
	//	m_pcbMappedGameObject->m_nMaterial = m_pMaterial->m_nReflection;
	
	//m_pd3dcbGameObject->CopyData(1, *m_pcbMappedGameObject);
}

void GameObject::Animate(float fTimeElapsed)
{
}

void GameObject::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera)
{
}

void GameObject::SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList)
{
	//pd3dCommandList->SetGraphicsRootConstantBufferView(1, m_d3dCbvGPUDescriptorHandle.ptr);
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_nRootIndex, m_d3dCbvGPUDescriptorHandle);
}

void GameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, Camera *pCamera)
{
	OnPrepareRender(pd3dCommandList, pCamera);

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	SetRootParameter(pd3dCommandList);

	if (!m_ppMeshes.empty())
	{
		for (UINT i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}

void GameObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, UINT nObject, Camera * pCamera)
{
	OnPrepareRender(pd3dCommandList, pCamera);

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	SetRootParameter(pd3dCommandList);

	if (!m_ppMeshes.empty())
	{
		for (UINT i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) {
				m_ppMeshes[i]->Render(pd3dCommandList, nObject);
			}
		}
	}
}


void GameObject::ReleaseUploadBuffers()
{
	if (!m_ppMeshes.empty())
	{
		for (UINT i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void GameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObject::SetLookAt(XMFLOAT3& xmf3Target)
{
	XMFLOAT3 xmf3Up;
	if (!m_bLockRotateXZ) {
		xmf3Up = GetUp();
	}
	else {
		xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = Vector3::Subtract(xmf3Target, xmf3Position, true);
	XMFLOAT3 xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);

	if (!m_bLockRotateXZ) {
		xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);
	}

	m_xmf4x4World._11 = xmf3Right.x; m_xmf4x4World._12 = xmf3Right.y; m_xmf4x4World._13 = xmf3Right.z;
	m_xmf4x4World._21 = xmf3Up.x; m_xmf4x4World._22 = xmf3Up.y; m_xmf4x4World._23 = xmf3Up.z;
	m_xmf4x4World._31 = xmf3Look.x; m_xmf4x4World._32 = xmf3Look.y; m_xmf4x4World._33 = xmf3Look.z;
}

XMFLOAT3 GameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 GameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 GameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 GameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void GameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::MoveDir(XMFLOAT3 dir, float fDist)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = dir;
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDist);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void GameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void GameObject::SetScale(float value)
{
	m_xmf4x4World._11 *= value;
	m_xmf4x4World._22 *= value;
	m_xmf4x4World._33 *= value;
}

inline Status * GameObject::GetStatus() { return m_status; }
