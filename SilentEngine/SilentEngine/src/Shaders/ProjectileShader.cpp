#include "stdafx.h"
#include "ProjectileShader.h"



void ProjectileShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_EFFECT_INFO cEffectBuffer;
	m_ActiveBullet = 0;
	int index = 0;
	for (unsigned int i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->isLive()) {
			XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
			//XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
			cBuffer.m_nMaterial = 0;
			m_ObjectCB->CopyData(index, cBuffer);
			index += 1;
			m_ActiveBullet += 1;
		}
	}

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		cEffectBuffer.m_nMaxXcount = (UINT)m_fMaxXCount;
		cEffectBuffer.m_nNowXcount = (UINT)m_fNowXCount;

		cEffectBuffer.m_nMaxYcount = (UINT)m_fMaxYCount;
		cEffectBuffer.m_nNowYcount = (UINT)m_fNowYCount;

		m_EffectCB->CopyData(i, cEffectBuffer);
	}

	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_MATERIAL, m_MatCB->Resource()->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_LIGHTS, m_LightsCB->Resource()->GetGPUVirtualAddress());
}

void ProjectileShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_nObjects = 10;

	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSEffect", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSEffect", "ps_5_0");

	CTexture *pTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\blackLight.dds", 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"res\\Texture\\blackLight_n.dds", 1);

	m_fMaxXCount = 4.0f;
	m_fMaxYCount = 4.0f;

	unsigned int i = 0;

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 4);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, i++, sizeof(CB_GAMEOBJECT_INFO), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_EffectCB->Resource(), 4, i++, sizeof(CB_EFFECT_INFO), false);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, 2, true);

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(pTexture);
	m_pMaterial->SetReflection(1);

	i = 0;
	CBoardMeshIlluminatedTextured *pBoard = new CBoardMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 25.0f, 25.0f, 0.0f);

	m_ppObjects = vector<GameObject*>(m_nObjects);


	Bullet* pInstnaceObject = new Bullet();
	pInstnaceObject->SetMesh(0, pBoard);
	pInstnaceObject->SetPosition(0,0,0);
	pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	pInstnaceObject->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (i + 1)));
	pInstnaceObject->SetInstanceCount(m_nObjects);

	m_ppObjects[i++] = pInstnaceObject;

	for (int i=1; i < m_nObjects; ++i) {
		Bullet* pGameObjects = new Bullet();
		pGameObjects->SetPosition(0,0,0);
		m_ppObjects[i] = pGameObjects;
	}
}


void ProjectileShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	Shaders::Render(pd3dCommandList, pCamera);

	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

	if (m_ppObjects[0])
		m_ppObjects[0]->Render(pd3dCommandList, pCamera);
}

void ProjectileShader::Animate(float fTimeElapsed)
{
	BillboardShader::Animate(fTimeElapsed);
	for (int i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->isLive()) {
			m_ppObjects[i]->Animate(fTimeElapsed);
		}
	}
}

void ProjectileShader::Shoot(XMFLOAT3 myPos, XMFLOAT3 targetPos)
{
	reinterpret_cast<Bullet*>(m_ppObjects[m_now])->Shoot(myPos, targetPos);
	//(Bullet*)(m_ppObjects[m_now]).Shoot(myPos, targetPos);
	m_now += 1;
	if (m_now >= m_nObjects)
		m_now = 0;
}
