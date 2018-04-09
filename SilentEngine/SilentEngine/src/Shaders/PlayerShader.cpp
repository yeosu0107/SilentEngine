#include "stdafx.h"
#include "PlayerShader.h"
#include "..\Object\Player.h"


PlayerShader::PlayerShader(int index, Camera* camera) :
	DynamicModelShader(index)
{
	m_myCamera = camera;
}

PlayerShader::~PlayerShader()
{
}

void PlayerShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, int nRenderTargets, void * pContext)
{
	m_nPSO = 2;

	CreatePipelineParts();

	m_VSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModel", "vs_5_0");
	m_PSByteCode[0] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicModel", "ps_5_0");

	m_VSByteCode[1] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Shadow.hlsl", nullptr, "VSShadowMap", "vs_5_0");
	m_PSByteCode[1] = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\Shadow.hlsl", nullptr, "PSShadowMap", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));

	CreateGraphicsRootSignature(pd3dDevice);

	//BuildPSO(pd3dDevice, 0, PSO_SHADOWMAP);
	BuildPSO(pd3dDevice, nRenderTargets, PSO_OBJECT);

	if (globalModels->isMat(modelIndex)) {
		CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, globalModels->getMat(modelIndex).c_str(), 0);
		CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 4, false);

		m_pMaterial = new CMaterial();
		m_pMaterial->SetTexture(pTexture);
		m_pMaterial->SetReflection(1);
	}

	Player* tmp = new Player(globalModels->getModel(modelIndex), pd3dDevice, pd3dCommandList);
	tmp->SetAnimations(globalModels->getAnimCount(modelIndex), globalModels->getAnim(modelIndex));
	tmp->SetPhysController((BasePhysX*)pContext, tmp->getCollisionCallback(), &PxExtendedVec3(0, -170, 0));
	
	if (m_myCamera)
		tmp->SetCamera(m_myCamera, (BasePhysX*)pContext);
	tmp->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	m_ppObjects[0] = tmp;
}

void PlayerShader::Animate(float fTimeElapsed)
{
	ModelShader::Animate(fTimeElapsed);
}
