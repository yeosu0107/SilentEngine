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

	vector<TextureDataForm> texutredata(1);
	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameExit.dds";

	m_pButtons = make_unique<UIButtonShaders>();
	m_pButtons->BuildObjects(pDevice, pCommandList, 1, &texutredata);
	m_pButtons->SetAlpha(0.0f, 0);
	m_pButtons->SetPosScreenRatio(XMFLOAT2(0.5f, 2.25 / 9.0f));
}

void GameOverScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pBlocks->Render(pCommandList);
	m_pButtons->Render(pCommandList);
}

void GameOverScene::Reset()
{
	m_pButtons->SetAlpha(0.0f, 0);
	
	m_Anitype = CLOSE_FIRST;

	m_pBlocks->SetPosScreenRatio(XMFLOAT2(m_BlockStartMovePos, 0.5f), 0);
	m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - m_BlockStartMovePos, 0.5f), 1);

	m_ProgessTime = 0.0f;
}

int GameOverScene::CollisionToMouse(POINT * mousePos)
{
	// return 0 : Restart , 1 : Exit
	m_pButtons->SetPoint(mousePos);
	return m_pButtons->CollisionButton();
}

bool GameOverScene::Update(const Timer & gt)
{
	float fConvertProgressTime = m_ProgessTime;
	float fMoveRatio = 0.0f;
	
	m_ProgessTime += gt.DeltaTime();

	switch (m_Anitype) {
	case CLOSE_FIRST:
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * 0.2f));
		fMoveRatio = D3DMath::Lerp(m_BlockStartMovePos, m_MidPos, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);

		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = RE_OPEN;
			m_ProgessTime = 0.0f;
		}
		break;

	case RE_OPEN: 
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * 0.4f));
		fMoveRatio = D3DMath::Lerp(m_MidPos, m_ReOpenRatio, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);

		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = CLOSE_SECOND;
			m_ProgessTime = 0.0f;
		}
		break;

	case CLOSE_SECOND:
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * 0.2f));
		fMoveRatio = D3DMath::Lerp(m_ReOpenRatio, m_BlockMaxMovePos, fConvertProgressTime);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(fMoveRatio, 0.5f), 0);
		m_pBlocks->SetPosScreenRatio(XMFLOAT2(1.0f - fMoveRatio, 0.5f), 1);

		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = APPEAR_BOTTON;
			m_ProgessTime = 0.0f;
		}
		break;

	case APPEAR_BOTTON:
		fConvertProgressTime = min(1.0, fConvertProgressTime / (m_TotalAnimationTime * 0.2f));
		fMoveRatio = D3DMath::Lerp(0.0f, 1.0f, fConvertProgressTime);
		m_pButtons->SetAlpha(fMoveRatio, 0);
		
		if (fConvertProgressTime >= 1.0f) {
			m_Anitype = Animate_None;
			m_ProgessTime = 0.0f;
		}
		break;
	}

	return false;
}
