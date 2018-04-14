#pragma once

#include "..\Model\ModelShader.h"
#include "..\Model\InstanceModelShader.h"

template<class T>
class EnemyShader : public DynamicModelShader
{
private:

public:
	EnemyShader(int index) : DynamicModelShader(index) { }
	~EnemyShader() { }

	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets = 1, void * pContext = NULL) {
		ModelLoader* globalModels = GlobalVal::getInstance()->getModelLoader();
		
		m_nPSO = 2;
		CreatePipelineParts();

		m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModel", "vs_5_0");
		m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicModel", "ps_5_0");

		m_VSByteCode[1] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModel", "vs_5_0");
		m_PSByteCode[1] = nullptr;

		m_nObjects = 1;
		m_ppObjects = vector<GameObject*>(m_nObjects);

		CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, 1);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
		CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));

		CreateGraphicsRootSignature(pd3dDevice);

		BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
		BuildPSO(pd3dDevice, nRenderTargets);

		if (globalModels->isMat(modelIndex)) {
			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
			pTexture->AddTexture(ShadowShader->Rsc(), ShadowShader->UploadBuffer(), RESOURCE_TEXTURE2D_SHADOWMAP);
			CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, true);

			m_pMaterial = new CMaterial();
			m_pMaterial->SetTexture(pTexture);
			m_pMaterial->SetReflection(1);
		}


		for (UINT i = 0; i < m_nObjects; ++i) {
			T* t_enemy = new T(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
			t_enemy->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
			t_enemy->SetPosition(XMFLOAT3(50 + (i * 50), -170, 50));
			t_enemy->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
			m_ppObjects[i] = t_enemy;
		}
	}

	void setPhys(BasePhysX* phys) {
		
		for (auto& p : m_ppObjects) {
			T* tmp = reinterpret_cast<T*>(p);
			tmp->SetPhysController(phys, tmp->getCollisionCallback(), &XMtoPXEx(tmp->GetPosition()));
		}
	}

	void releasePhys() {
		for (auto& p : m_ppObjects) {
			if(p->isLive())
				reinterpret_cast<T*>(p)->releasePhys();
		}
	}
};

/*emplate<class T>
class EnemyShader : public InstanceDynamicModelShader
{
private:
	string * name = new string("Enemy01");
public:
	EnemyShader(int index) : InstanceDynamicModelShader(index) { }
	~EnemyShader() { }

	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) {

		m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicInstanceModel", "vs_5_0");
		m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicInstanceModel", "ps_5_0");

		m_nObjects = 5;
		m_ppObjects = vector<GameObject*>(m_nObjects);


		CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 2);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
		CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_BoneCB->Resource(), 1, false);

		CreateGraphicsRootSignature(pd3dDevice);
		BuildPSO(pd3dDevice, nRenderTargets);

		if (globalModels->isMat(modelIndex)) {
			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
			CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, 1, false);

			m_pMaterial = new CMaterial();
			m_pMaterial->SetTexture(pTexture);
			m_pMaterial->SetReflection(1);
		}

		for (UINT i = 0; i < m_nObjects; ++i) {
			T* t_enemy = new T(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
			t_enemy->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
			t_enemy->SetPosition(XMFLOAT3(50 + (i * 10), -170, 50));
			t_enemy->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
			m_ppObjects[i] = t_enemy;
		}
	}

	void setPhys(BasePhysX* phys) {
		for (auto& p : m_ppObjects) {
			T* tmp = reinterpret_cast<T*>(p);
			tmp->SetPhysController(phys, tmp->getCollisionCallback(), &XMtoPXEx(tmp->GetPosition()), name);
		}
	}

	void releasePhys() {
		for (auto& p : m_ppObjects) {
			reinterpret_cast<T*>(p)->releasePhys();
		}
	}
};*/