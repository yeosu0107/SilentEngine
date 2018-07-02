#pragma once

#include "..\Model\ModelShader.h"
#include "..\Model\InstanceModelShader.h"

template<class T>
class EnemyShader : public DynamicModelShader
{
private:
	UINT m_remainEnemy;
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

		m_nObjects = (int)(pContext);
		m_ppObjects = vector<GameObject*>(m_nObjects);

		CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, 1 + NUM_DIRECTION_LIGHTS);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);
		CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));

		CreateGraphicsRootSignature(pd3dDevice);

		BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
		BuildPSO(pd3dDevice, nRenderTargets);

		if (globalModels->isMat(modelIndex)) {
			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
			for (int i = 0; i < NUM_DIRECTION_LIGHTS; ++i)
				pTexture->AddTexture(ShadowShader->Rsc(i), ShadowShader->UploadBuffer(i), RESOURCE_TEXTURE2D_SHADOWMAP);

			CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, true);

			m_pMaterial = new CMaterial();
			m_pMaterial->SetTexture(pTexture);
			m_pMaterial->SetReflection(1);
		}


		for (UINT i = 0; i < m_nObjects; ++i) {
			T* t_enemy = new T(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
			t_enemy->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
			//t_enemy->SetPosition(XMFLOAT3(50 + (i * 50), -170, 50 + (i * 30)));
			t_enemy->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
			m_ppObjects[i] = t_enemy;
		}
		m_remainEnemy = m_nObjects;
	}

	void setPhys(BasePhysX* phys) {
		int i = 0;
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

	void setPosition(XMFLOAT3* pos) {
		for (int i = 0; i < m_nObjects; ++i) {
			T* tmp = reinterpret_cast<T*>(m_ppObjects[i]);
			tmp->reset();
			tmp->teleport(pos[i]);
		}
		m_remainEnemy = m_nObjects;
	}

	UINT getRemainObjects() const { 
		return m_remainEnemy; 
	}
};

//template<class T>
//class EnemyShader : public InstanceDynamicModelShader
//{
//private:
//	string * name = new string("Enemy01");
//public:
//	EnemyShader(int index) : InstanceDynamicModelShader(index) { }
//	~EnemyShader() { }
//
//	void Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
//	{
//		Shaders::OnPrepareRender(pd3dCommandList);
//		Shaders::Render(pd3dCommandList, pCamera);
//
//		if (m_pMaterial)
//			m_pMaterial->UpdateShaderVariables(pd3dCommandList);
//
//		if (m_ppObjects[0]) {
//		
//			m_ppObjects[0]->Render(pd3dCommandList, m_nObjects, pCamera, true);
//		}
//	}
//
//	virtual void BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext) {
//		ModelLoader* globalModels = GlobalVal::getInstance()->getModelLoader();
//		
//		m_nPSO = 2;
//		CreatePipelineParts();
//
//		m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicInstanceModel", "vs_5_1");
//		m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicInstanceModel", "ps_5_1");
//
//		m_VSByteCode[1] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicInstanceModel", "vs_5_1");
//		m_PSByteCode[1] = nullptr;
//
//		m_nObjects = 1;
//		m_ppObjects = vector<GameObject*>(m_nObjects);
//
//		CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 3);
//		CreateShaderVariables(pd3dDevice, pd3dCommandList);
//		CreateInstanceShaderResourceViews(pd3dDevice, pd3dCommandList, m_BoneCB->Resource(), 1, false);
//
//		CreateGraphicsRootSignature(pd3dDevice);
//
//		BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
//		BuildPSO(pd3dDevice, nRenderTargets);
//
//		if (globalModels->isMat(modelIndex)) {
//			CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
//			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
//			pTexture->AddTexture(ShadowShader->Rsc(), ShadowShader->UploadBuffer(), RESOURCE_TEXTURE2D_SHADOWMAP);
//
//			CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, 1, true);
//
//			m_pMaterial = new CMaterial();
//			m_pMaterial->SetTexture(pTexture);
//			m_pMaterial->SetReflection(1);
//		}
//
//		int i = 0;
//
//		T* t_enemy = new T(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
//		t_enemy->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
//		t_enemy->SetPosition(XMFLOAT3(50 + (i * 10), -170, 50));
//		t_enemy->SetCbvGPUDescriptorHandlePtr(m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
//		m_ppObjects[i++] = t_enemy;
//		
//		for (; i < m_nObjects; ++i) {
//			T* instenemy = new T(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
//			instenemy->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
//			instenemy->SetPosition(XMFLOAT3(50 + (i * 10), -170, 50));
//			instenemy->SetCbvGPUDescriptorHandlePtr(m_d3dSrvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
//			m_ppObjects[i] = instenemy;
//		}
//	}
//
//	void setPhys(BasePhysX* phys) {
//		for (auto& p : m_ppObjects) {
//			T* tmp = reinterpret_cast<T*>(p);
//			tmp->SetPhysController(phys, tmp->getCollisionCallback(), &XMtoPXEx(tmp->GetPosition()));
//		}
//	}
//
//	void releasePhys() {
//		for (auto& p : m_ppObjects) {
//			reinterpret_cast<T*>(p)->releasePhys();
//		}
//	}
//};