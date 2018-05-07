#include "stdafx.h"
#include "UIObject.h"


UIObject::UIObject()
{
}

UIObject::~UIObject()
{
}

void UIObject::SetRootParameter(ID3D12GraphicsCommandList * pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_d3dCbvGPUDescriptorHandle);
}

void UIObject::Render(ID3D12GraphicsCommandList * pd3dCommandList)
{
	SetRootParameter(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void UIObject::SetScreenSize(XMFLOAT2 & size)
{
	m_xmf2ScreenSize = size;
}

void UIObject::SetPosition(XMFLOAT2 & pos)
{
	m_xmf2ScreenPos = pos;
}

void UIObject::SetScale(XMFLOAT2 & scale)
{
	m_xmf2Scale = scale;
}

void UIObject::SetSize(XMUINT2 & size)
{
	m_nSize = size;
}

void UIObject::SetNumSprite(XMUINT2 & numSprite, XMUINT2& nowSprite)
{
	m_nNumSprite = numSprite;
	m_nNowSprite = nowSprite;
}

///////////////////////////////////////////////////


void HPBarObject::SetPlayerStatus(Status * pPlayerStatus)
{
	m_pPlayerStatus = pPlayerStatus;
	m_fMaxHP = static_cast<float>(pPlayerStatus->m_health);
	m_fLerpHP = m_fMaxHP;
}

void HPBarObject::Update(float fTimeElapsed)
{
	float currHP = m_pPlayerStatus->m_health;

	m_fLerpHP = currHP + (m_fLerpHP - currHP) * fTimeElapsed;

	m_fData = currHP / m_fMaxHP;
}
