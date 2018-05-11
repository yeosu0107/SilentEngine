#pragma once

#include "Shaders.h"
#include "..\Object\InstanceObject.h"

template<class T>
class PaticleShader : public BillboardShader
{
private:
	UINT m_ActivePaticle;
public:
	PaticleShader() : BillboardShader() {
		m_ActivePaticle = 0;
	}
	~PaticleShader() {}

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

				cEffectBuffer.m_nMaxXcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fMaxXCount;
				cEffectBuffer.m_nNowXcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fNowXCount;

				cEffectBuffer.m_nMaxYcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fMaxYCount;
				cEffectBuffer.m_nNowYcount = (UINT)reinterpret_cast<T*>(m_ppObjects[i])->m_fNowYCount;

				m_EffectCB->CopyData(index, cEffectBuffer);

				index += 1;
				m_ActivePaticle += 1;
			}
		}

		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_MATERIAL, m_MatCB->Resource()->GetGPUVirtualAddress());
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOTPARAMETER_LIGHTS, m_LightsCB->Resource()->GetGPUVirtualAddress());
	}

	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) {
		m_nObjects = 10;

		m_nPSO = 1;

		CreatePipelineParts();

		m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "VSEffect", "vs_5_0");
		m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Effect.hlsl", nullptr, "PSEffect", "ps_5_0");

		TextureDataForm* mtexture = (TextureDataForm*)pContext;

		CTexture *pTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_texture.c_str(), 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, mtexture->m_normal.c_str(), 1);

		UINT i = 0;

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
		CBoardMeshIlluminatedTextured *pBoard = new CBoardMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f);

		m_ppObjects.resize(m_nObjects);

		T* pInstnaceObject = new T();
		pInstnaceObject->SetMesh(0, pBoard);

		pInstnaceObject->m_fMaxXCount = mtexture->m_MaxX;
		pInstnaceObject->m_fMaxYCount = mtexture->m_MaxY;
		pInstnaceObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		pInstnaceObject->SetEffectCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * (i + 1)));

		m_ppObjects[i++] = pInstnaceObject;

		for (UINT i = 1; i < m_nObjects; ++i) {
			T* pGameObjects = new T();

			pGameObjects->m_fMaxXCount = mtexture->m_MaxX;
			pGameObjects->m_fMaxYCount = mtexture->m_MaxY;
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
};