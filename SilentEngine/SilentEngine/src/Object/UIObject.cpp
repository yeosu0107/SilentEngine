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

bool UIObject::CollisionUI(POINT * pPoint, float trueSetData, float falseSetData)
{
	if (m_xmf2StartPos.x < pPoint->x && m_xmf2StartPos.y > pPoint->y) {
		if (m_xmf2EndPos.x > pPoint->x && m_xmf2EndPos.y < pPoint->y) {
			m_fData = trueSetData;
			return true;
		}
	}
	m_fData = falseSetData;
	return false;
}

void UIObject::CreateCollisionBox()
{
	m_xmf2StartPos = XMFLOAT2(
		m_xmf2ScreenPos.x - static_cast<float>(m_nSize.x / 2) * m_xmf2Scale.x,
		static_cast<float>(FRAME_BUFFER_HEIGHT)-(m_xmf2ScreenPos.y - static_cast<float>(m_nSize.y / 2) * m_xmf2Scale.y)
	);
	m_xmf2EndPos = XMFLOAT2(
		m_xmf2ScreenPos.x + static_cast<float>(m_nSize.x / 2) * m_xmf2Scale.x,
		static_cast<float>(FRAME_BUFFER_HEIGHT)-(m_xmf2ScreenPos.y + static_cast<float>(m_nSize.y / 2) * m_xmf2Scale.y)
	);
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
