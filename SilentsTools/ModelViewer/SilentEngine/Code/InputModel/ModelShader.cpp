#include "stdafx.h"
#include "ModelShader.h"


ModelShader::ModelShader()
{
}


ModelShader::~ModelShader()
{
}

D3D12_INPUT_LAYOUT_DESC ModelShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,			0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0,			12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0,				24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "BORNINDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT, 0,		32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "WEIGHT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,		48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

void ModelShader::CreateShader(ID3D12Device * pd3dDevice, ID3D12RootSignature * pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

D3D12_SHADER_BYTECODE ModelShader::CreateVertexShader(ID3DBlob ** ppd3dShaderBlob)
{
	return (CShader::CompileShaderFromFile(L"./hlsl/Shaders.hlsl", "VSModel", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ModelShader::CreatePixelShader(ID3DBlob ** ppd3dShaderBlob)
{
	return (CShader::CompileShaderFromFile(L"./hlsl/Shaders.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", ppd3dShaderBlob));
}

void ModelShader::BuildObjects(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, void * pContext)
{
	m_nObjects = 1;

	CTexture *pTexture = new CTexture(1, RESOURCE_TEXTURE2DARRAY, 0);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"snow.DDS", 0);

	UINT ncbElementBytes = ((sizeof(CB_OBJECT_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, m_nObjects, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	//CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, ROOT_PARAMETER_TEXTURE, false);

#ifdef _WITH_BATCH_MATERIAL
	//m_pMaterial = new CMaterial();
	//m_pMaterial->SetTexture(pTexture);
	//m_pMaterial->SetReflection(1);
#else
	CMaterial *pCubeMaterial = new CMaterial();
	pCubeMaterial->SetTexture(pTexture);
	pCubeMaterial->SetReflection(1);
#endif
	LoadModel* model= new LoadModel("angle.FBX");
	model->SetMeshes(pd3dDevice, pd3dCommandList);

	m_ppObjects = new CGameObject*[m_nObjects];

	for (int i = 0; i < m_nObjects; ++i) {
		ModelObject* object = new ModelObject(model, pd3dDevice, pd3dCommandList);
		object->SetPosition(i * 10, 0, 0);
		object->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i] = object;

	}
}

void ModelShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) delete m_pMaterial;
#endif
}

void ModelShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void ModelShader::CreateShaderVariables(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_OBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
}

void ModelShader::UpdateShaderVariables(ID3D12GraphicsCommandList * pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_OBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		//위치
		CB_OBJECT_INFO *pbMappedcbGameObject = (CB_OBJECT_INFO *)((UINT8 *)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));

		//뼈
		::memcpy(pbMappedcbGameObject->m_bone, m_ppObjects[j]->GetBoneData(), sizeof(XMFLOAT4X4) * m_ppObjects[j]->GetBoneNum());

		//메터리얼
#ifdef _WITH_BATCH_MATERIAL
		if (m_pMaterial) 
			pbMappedcbGameObject->m_nMaterial = m_pMaterial->m_nReflection;
#endif
	}
}

void ModelShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}
	CShader::ReleaseShaderVariables();
}

void ModelShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) 
			if (m_ppObjects[j]) 
				m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

void ModelShader::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
#endif

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) 
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}
