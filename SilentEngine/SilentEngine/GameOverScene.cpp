#include "stdafx.h"
#include "GameOverScene.h"


GameOverScene::GameOverScene()
{
}


GameOverScene::~GameOverScene()
{
}

void GameOverScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Anitype = CLOSE_FIRST;

	m_pBlocks = make_unique<UIFullScreenShaders>();
	m_pBlocks->BuildObjects(pDevice, pCommandList, 1);
}

void GameOverScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pBlocks->Render(pCommandList);
}

bool GameOverScene::Update(const Timer & gt)
{
	float fConvertProgressTime = m_ProgessTime;
	float fMoveRatio = 0.0f;

	m_ProgessTime += gt.DeltaTime();

	switch (m_Anitype) {
	case CLOSE_FIRST:
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * m_AnimationRatio * 0.125f));
		fMoveRatio = D3DMath::Lerp(m_BlockStartMovePos, m_BlockMaxMovePos, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);

		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = RE_OPEN;
			m_ProgessTime = 0.0f;
		}
		break;

	case RE_OPEN: 
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * m_AnimationRatio * 0.5f));
		fMoveRatio = D3DMath::Lerp(m_BlockMaxMovePos, m_ReOpenRatio, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);

		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = CLOSE_SECOND;
			m_ProgessTime = 0.0f;
		}
		break;

	case CLOSE_SECOND:
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * m_AnimationRatio * 0.375f));
		fMoveRatio = D3DMath::Lerp(m_ReOpenRatio, m_BlockMaxMovePos, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);
		break;
	}

	return false;
}
