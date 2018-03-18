#include "stdafx.h"
#include "InstanceObject.h"
#include "GameObjects.h"


InstanceObject::InstanceObject()
{
}


InstanceObject::~InstanceObject()
{
}

void InstanceObject::SetRootParameter(ID3D12GraphicsCommandList * pd3dCommandList)
{
	// ù ���ڴ� Instance�����Ͱ� ����ִ� �ε��� 
	pd3dCommandList->SetGraphicsRootDescriptorTable(1, m_d3dCbvGPUDescriptorHandle);
}

void InstanceObject::SetInstanceDataResource(ComPtr<ID3D12Resource> pResource)
{
	m_pInstanceDataResource = pResource;
}

void InstanceObject::Render(ID3D12GraphicsCommandList * pd3dCommandList, Camera * pCamera)
{
	OnPrepareRender(pd3dCommandList, pCamera);

	SetRootParameter(pd3dCommandList);

	if (!m_ppMeshes.empty())
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->InstanceRender(pd3dCommandList, m_nInstanceCount);
		}
	}
}


void InstanceObject::SetInstanceCount(UINT nInstanceCount)
{
	m_nInstanceCount = nInstanceCount;
}
