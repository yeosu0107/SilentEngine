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

void PlayerShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_VSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "VSDynamicModel", "vs_5_0");
	m_PSByteCode = COMPILEDSHADERS->GetCompiledShader(L"hlsl\\model.hlsl", nullptr, "PSDynamicModel", "ps_5_0");

	m_nObjects = 1;
	m_ppObjects = vector<GameObject*>(m_nObjects);


	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_BoneCB->Resource(), D3DUtil::CalcConstantBufferByteSize(sizeof(CB_DYNAMICOBJECT_INFO)));

	CreateGraphicsRootSignature(pd3dDevice);
	BuildPSO(pd3dDevice);

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
	//tmp->SetPosition(XMFLOAT3(0, -170, 0));
	tmp->SetPhysController(globalPhysX->getCapsuleController(XMtoPXEx(XMFLOAT3(0, -170, 0)) ,tmp->getCollisionCallback()));
	if (m_myCamera)
		tmp->SetCamera(m_myCamera);
	tmp->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * 0));
	m_ppObjects[0] = tmp;
}

void PlayerShader::Animate(float fTimeElapsed)
{
	ModelShader::Animate(fTimeElapsed);
}
