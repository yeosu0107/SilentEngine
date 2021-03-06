#include "stdafx.h"
#include "ProjectileShader.h"

void ProjectileShader::SetType(UINT* ntype, UINT num) {
	UINT now = 0;

	for (auto& p : m_ppObjects) {
		if (p->isLive())
			continue;
		reinterpret_cast<Bullet*>(p)->m_nType = ntype[now];
		reinterpret_cast<Bullet*>(p)->m_fMaxXCount = m_TextureDatas[ntype[now]].m_MaxX;
		reinterpret_cast<Bullet*>(p)->m_fMaxYCount = m_TextureDatas[ntype[now]].m_MaxY;
		now += 1;
		if (now == num)
			break;
	}
}

void ProjectileShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	CB_GAMEOBJECT_INFO cBuffer;
	CB_EFFECT_INFO cEffectBuffer;
	m_ActiveBullet = 0;
	int index = 0;
	Camera* pCamera = m_pCamera;

	for (unsigned int i = 0; i < m_nObjects; ++i) {
		if (m_ppObjects[i]->isLive()) {
			XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
			cBuffer.m_nMaterial = 0;
			m_ObjectCB->CopyData(index, cBuffer);

			cEffectBuffer.m_nMaxXcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fMaxXCount;
			cEffectBuffer.m_nNowXcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fNowXCount;

			cEffectBuffer.m_nMaxYcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fMaxYCount;
			cEffectBuffer.m_nNowYcount = (UINT)reinterpret_cast<Bullet*>(m_ppObjects[i])->m_fNowYCount;

			cEffectBuffer.m_nType = reinterpret_cast<Bullet*>(m_ppObjects[i])->m_nType;
			m_EffectCB->CopyData(index, cEffectBuffer);

			index += 1;
			m_ActiveBullet += 1;
		}
	}
}

void ProjectileShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nObjects = 60;

	m_nPSO = 1;
	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSEffect", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS.GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSEffect", "ps_5_0");

	int size = reinterpret_cast<vector<TextureDataForm>*>(pContext)->size();

	m_TextureDatas.resize(size);
	vector<TextureDataForm> mtexture = *reinterpret_cast<vector<TextureDataForm>*>(pContext);

	CTexture *pTexture = new CTexture(size, RESOURCE_TEXTURE2D, 0);

	for (int i = 0; i < size; ++i) {
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture[i].m_texture.c_str(), i);
		m_TextureDatas[i] = (*reinterpret_cast<vector<TextureDataForm>*>(pContext))[i];
	}

	unsigned int i = 0;

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 2 + size);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_ObjectCB->Resource(), 1, i++, sizeof(CB_GAMEOBJECT_INFO), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_EffectCB->Resource(), 4, i++, sizeof(CB_EFFECT_INFO), false);
	CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 3, 2, true);

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
	pInstnaceObject->m_fMaxXCount = mtexture[0].m_MaxX;
	pInstnaceObject->m_fMaxYCount = mtexture[0].m_MaxY;
	pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
	pInstnaceObject->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (i + 1)));

	m_ppObjects[i++] = pInstnaceObject;

	for (UINT i=1; i < m_nObjects; ++i) {
		Bullet* pGameObjects = new Bullet();
		pGameObjects->SetMesh(0, pBoard);
		pGameObjects->SetPosition(0,0,0);
		pGameObjects->m_fMaxXCount = mtexture[0].m_MaxX;
		pGameObjects->m_fMaxYCount = mtexture[0].m_MaxY;
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

void ProjectileShader::Shoot(XMFLOAT3 myPos, XMFLOAT3 targetPos)
{
	UINT start = m_now;
	while (m_ppObjects[m_now]->isLive()) {
		m_now += 1;
		if (m_now == start)
			return;
		if (m_now >= m_nObjects) {
			m_now = 0;
			return;
		}
	}
	reinterpret_cast<Bullet*>(m_ppObjects[m_now])->Shoot(myPhys, myPos, targetPos);
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
		if(p->isLive())
			reinterpret_cast<Bullet*>(p)->releasePhys();
		p->SetLive(false);
	}
	m_now = 0;
}
