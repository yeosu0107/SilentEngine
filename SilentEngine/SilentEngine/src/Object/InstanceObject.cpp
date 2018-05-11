#include "stdafx.h"
#include "InstanceObject.h"
#include "GameObjects.h"


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

void EffectInstanceObject::Animate(float fTimeElapsed)
{
	m_fNowXCount += fTimeElapsed * m_fAnimationSpeed;
	if (m_fNowXCount >= m_fMaxXCount) {
		m_fNowXCount = 0.0f;
		m_fNowYCount += 1.0f;
		if (m_fNowYCount >= m_fMaxYCount) {
			m_fNowYCount = 0.0f;
		}
	}
}

PaticleObject::PaticleObject()
{
	m_live = false;
	m_fAnimationSpeed = 150.0f;
	isLoop = false;
}

void PaticleObject::Animate(float fTimeElapsed)
{
	EffectInstanceObject::Animate(fTimeElapsed);
	if (isLoop)
		return;
	if (m_fNowXCount == 0 && m_fNowYCount == 0)
		m_live = false;
}

void PaticleObject::SetPosition(XMFLOAT3 xmf3Position)
{
	GameObject::SetPosition(xmf3Position);
	Rotate(&GetLook(), rand() % 350);
	m_live = true;
}

HitPaticle::HitPaticle() : PaticleObject() { }

void HitPaticle::SetPosition(XMFLOAT3 xmf3Position)
{
	PaticleObject::SetPosition(xmf3Position);
	
}
