#include "stdafx.h"
#include "ProjectileShader.h"

void ProjectileShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_EFFECT_INFO cEffectBuffer;
	m_ActiveBullet = 0;
	int index = 0;
	Camera* pCamera = m_pCamera;

	std::sort(m_ppObjects.begin(), m_ppObjects.end(), [pCamera](GameObject* a, GameObject* b) {

		Camera* pLamdaCamera = (Camera*)pCamera;
		float aLengthToCamera = Vector3::Length(Vector3::Subtract(pLamdaCamera->GetPosition(), a->GetPosition(), false));
		float bLengthToCamera = Vector3::Length(Vector3::Subtract(pLamdaCamera->GetPosition(),b->GetPosition(),false));
		return aLengthToCamera > bLengthToCamera; }
	);
	
	for (unsigned int i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->isLive()) {
			XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
			cBuffer.m_nMaterial = 0;
			m_ObjectCB->CopyData(index, cBuffer);

			cEffectBuffer.m_nMaxXcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fMaxXCount;
			cEffectBuffer.m_nNowXcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fNowXCount;

			cEffectBuffer.m_nMaxYcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fMaxYCount;
			cEffectBuffer.m_nNowYcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fNowYCount;

			m_EffectCB->CopyData(index, cEffectBuffer);

			index += 1;
			m_ActiveBullet += 1;
		}
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_MATERIAL, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_LIGHTS, m_LightsCB->Resource()->GetGPUVirtualAddress());
}

void ProjectileShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 10;

	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSEffect", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSEffect", "ps_5_0");

	TextureDataForm* mtexture = (TextureDataForm*)pContext;

	CTexture *pTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_texture.c_str(), 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_normal.c_str(), 1);

	unsigned int i = 0;

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 4);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, i++, sizeof(CB_GAMEOBJECT_INFO), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_EffectCB->Resource(), 4, i++, sizeof(CB_EFFECT_INFO), false);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, 2, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice, nRenderTargets);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	i = 0;
	CBoardMeshIlluminatedTextured *pBoard = new CBoardMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 25.0f, 25.0f, 0.0f);

	m_ppObjects = vector<GameObject*>(m_nObjects);


	Bullet* pInstnaceObject = new Bullet();
	pInstnaceObject->SetMesh(0, pBoard);
	pInstnaceObject->SetPosition(0,0,0);
	pInstnaceObject->m_fMaxXCount = mtexture->m_MaxX;
	pInstnaceObject->m_fMaxYCount = mtexture->m_MaxY;
	pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	pInstnaceObject->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (i + 1)));

	m_ppObjects[i++] = pInstnaceObject;

	for (int i=1; i < m_nObjects; ++i) {
		Bullet* pGameObjects = new Bullet();
		pGameObjects->SetMesh(0, pBoard);
		pGameObjects->SetPosition(0,0,0);
		pGameObjects->m_fMaxXCount = mtexture->m_MaxX;
		pGameObjects->m_fMaxYCount = mtexture->m_MaxY;
		pGameObjects->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize) * 0);
		pGameObjects->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (1)));

		m_ppObjects[i] = pGameObjects;
	}
}


void ProjectileShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, m_ActiveBullet, pCamera);
}

void ProjectileShader::Shoot(BasePhysX* phys, XMFLOAT3 myPos, XMFLOAT3 targetPos)
{
	UINT start = m_now;
	while (m_ppObjects[m_now]->isLive()) {
		m_now += 1;
		if (m_now == start)
			return;
		if (m_now >= m_nObjects) {
			m_now = 0;
		}
	}
	reinterpret_cast<Bullet*>(m_ppObjects[m_now])->Shoot(phys, myPos, targetPos);
	m_now += 1;
	if (m_now >= m_nObjects)
		m_now = 0;
}

XMFLOAT3* ProjectileShader::returnCollisionPos(UINT & num)
{
	UINT start = 0;
	for (auto& p : m_ppObjects) {
		Bullet* tmp = reinterpret_cast<Bullet*>(p);
		if (tmp->isCrash()) {
			m_crashes[start] = reinterpret_cast<Bullet*>(p)->getCrashPos();
			start += 1;
		}
	}
	num = start;

	return m_crashes;
}

void ProjectileShader::releasePhys()
{
	for (auto& p : m_ppObjects) {
		reinterpret_cast<Bullet*>(p)->releasePhys();
	}
}
