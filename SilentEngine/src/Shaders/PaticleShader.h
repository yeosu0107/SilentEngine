#pragma once

#include "Shaders.h"
#include "..\Object\InstanceObject.h"

template<class T>
class PaticleShader : public BillboardShader
{
private:
	UINT m_ActivePaticle;
	float paticleSize = 50.0f;
public:
	PaticleShader() : BillboardShader() {
		m_ActivePaticle = 0;
	}
	~PaticleShader() {}

	virtual void SetType(UINT* ntype, UINT num) {
		UINT now = 0;

		for (auto& p : m_ppObjects) {
			if (p->isLive())
				continue;
			reinterpret_cast<T*>(p)->m_nType = ntype[now];
			reinterpret_cast<T*>(p)->m_fMaxXCount = m_TextureDatas[ntype[now]].m_MaxX;
			reinterpret_cast<T*>(p)->m_fMaxYCount = m_TextureDatas[ntype[now]].m_MaxY;
			now += 1;
			if (now == num)
				break;
		}
	}

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {
		CB_GAMEOBJECT_INFO cBuffer;
		CB_EFFECT_INFO cEffectBuffer;
		m_ActivePaticle = 0;
		UINT index = 0;
		for (UINT i = 0; i < m_nObjects; ++i) {
			if (m_ppObjects[i]->isLive()) {
				XMStoreFloat4x4(&cBuffer.m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[i]->m_xmf4x4World)));
				cBuffer.m_nMaterial = 0;
				m_ObjectCB->CopyData(index, cBuffer);

				UINT type = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_nType;

				TextureDataForm form = m_TextureDatas[type];

				cEffectBuffer.m_nMaxXcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fMaxXCount;
				cEffectBuffer.m_nNowXcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fNowXCount;

				cEffectBuffer.m_nMaxYcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fMaxYCount;
				cEffectBuffer.m_nNowYcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fNowYCount;

				cEffectBuffer.m_nType = type;
				m_EffectCB->CopyData(index, cEffectBuffer);

				index += 1;
				m_ActivePaticle += 1;
			}
		}
	}

	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) {
		m_nObjects = 10;

		m_nPSO = 1;

		CreatePipelineParts();

		m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSEffect", "vs_5_0");
		m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSEffect", "ps_5_0");

		int size = reinterpret_cast<vector<TextureDataForm>*>(pContext)->size();

		m_TextureDatas.resize(size);
		vector<TextureDataForm> mtexture = *reinterpret_cast<vector<TextureDataForm>*>(pContext);

		CTexture *pTexture = new CTexture(size, RESOURCE_TEXTURE2D, 0);

		for (int i = 0; i < size; ++i) {
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture[i].m_texture.c_str(), i);
			m_TextureDatas[i] = (*reinterpret_cast<vector<TextureDataForm>*>(pContext))[i];
		}
		UINT i = 0;

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
		CBoardMeshIlluminatedTextured *pBoard = new CBoardMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, paticleSize, paticleSize, 0.0f);

		m_ppObjects.resize(m_nObjects);

		T* pInstnaceObject = new T();
		pInstnaceObject->SetMesh(0, pBoard);

		pInstnaceObject->m_fMaxXCount = mtexture[0].m_MaxX;
		pInstnaceObject->m_fMaxYCount = mtexture[0].m_MaxY;
		pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		pInstnaceObject->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (i + 1)));

		m_ppObjects[i++] = pInstnaceObject;

		for (UINT i = 1; i < m_nObjects; ++i) {
			T* pGameObjects = new T();

			pGameObjects->m_fMaxXCount = mtexture[0].m_MaxX;
			pGameObjects->m_fMaxYCount = mtexture[0].m_MaxY;
			pGameObjects->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize));
			pGameObjects->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 1));

			m_ppObjects[i] = pGameObjects;
		}
	}

	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
	{
		Shaders::Render(pd3dCommandList, pCamera);

		if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

		if (m_ppObjects[0])
			m_ppObjects[0]->Render(pd3dCommandList, m_ActivePaticle, pCamera);
	}

	virtual void SetPos(XMFLOAT3* getPos, UINT num) {
		UINT now = 0;

		for (auto& p : m_ppObjects) {
			if (p->isLive())
				continue;
			reinterpret_cast<T*>(p)->SetPosition(getPos[now]);
			
			now += 1;
			if (now == num)
				break;
		}
	}
	virtual void SetLive(const bool live, UINT num) {
		UINT now = 0;

		for (auto& p : m_ppObjects) {
			if (now < num)
				p->SetLive(live);
			else
				p->SetLive(!live);

			now += 1;
		}
	};

	void setLoop(bool loop) {
		for (auto& p : m_ppObjects)
			reinterpret_cast<T*>(p)->setLoop(loop);
	}

	void setPaticleSize(float size) { paticleSize = size; }
};