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
	// 첫 인자는 Instance데이터가 들어있는 인덱스 
	pd3dCommandList->SetGraphicsRootDescriptorTable(1, m_d3dCbvGPUDescriptorHandle);
}

EffectInstanceObject::EffectInstanceObject()
{
}

EffectInstanceObject::~EffectInstanceObject()
{
}

void EffectInstanceObject::SetRootParameter(ID3D12GraphicsCommandList * pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(1, m_d3dCbvGPUDescriptorHandle);
	pd3dCommandList->SetGraphicsRootDescriptorTable(4, m_d3dEffectCbvGPUDescriptorHandle);
}
