#include "stdafx.h"
#include "SceneState.h"

int virtualScene::CollisionToMouseClick(POINT * mousePos)
{
	m_pButtons->SetPoint(mousePos);
	return m_pButtons->ClickButton();
}

void virtualScene::ColiisionToMouseMove(POINT * mousePos)
{
	m_pButtons->SetPoint(mousePos);
	m_pButtons->CollisionButton();
}

void PauseScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	const float yStartPof = 555.0f;
	const float yOffset = 75.0f;
	int i = 0;

	vector<TextureDataForm> texutredata(4);

	////////////// HDR , BLOOM 글자 텍스쳐 출력 /////////////////
	texutredata[0] = { L"res\\MainSceneTexture\\BLOOM.DDS", L"", 1.0f, 1.0f };
	texutredata[1] = { L"res\\MainSceneTexture\\HDR.DDS", L"", 1.0f, 1.0f };

	m_pTextureUI[0] = make_unique<UIShaders>();
	m_pTextureUI[0]->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);
	m_pTextureUI[0]->SetPos(new XMFLOAT2(130.0f, yStartPof - (yOffset * i++)), 0);
	m_pTextureUI[0]->SetScale(&XMFLOAT2(0.8f, 0.8f), 0);

	m_pTextureUI[1] = make_unique<UIShaders>();
	m_pTextureUI[1]->BuildObjects(pDevice, pCommandList, 1, &texutredata[1]);
	m_pTextureUI[1]->SetPos(new XMFLOAT2(130.0f, yStartPof - (yOffset * i++)), 0);
	m_pTextureUI[1]->SetScale(&XMFLOAT2(0.8f, 0.8f), 0);

	i++;

	////////////// 컨티뉴 , 메인화면으로 버튼 /////////////////
	texutredata[0].m_texture = L"res\\Texture\\PauseGame.DDS";
	m_pPauseScreen = make_unique<TextureToFullScreen>();
	m_pPauseScreen->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);

	texutredata[0] = { L"res\\MainSceneTexture\\ONOFF.DDS", L"",	2.0f, 2.0f };
	texutredata[1] = { L"res\\MainSceneTexture\\ONOFF.DDS", L"",	2.0f, 2.0f };
	texutredata[2] = { L"res\\Texture\\Continue.DDS", L"",			2.0f, 1.0f };
	texutredata[3] = { L"res\\Texture\\BackToMainMenu.DDS", L"",	2.0f, 1.0f };

	m_pButtons = make_unique<UIButtonShaders>();
	m_pButtons->BuildObjects(pDevice, pCommandList, 1, &texutredata);
	m_pButtons->SetPos(new XMFLOAT2(350.0f, yStartPof - (yOffset * 0)), 0);
	m_pButtons->SetPos(new XMFLOAT2(350.0f, yStartPof - (yOffset * 1)), 1);
	m_pButtons->SetPos(new XMFLOAT2(130.0f, yStartPof - (yOffset * i++)), 2);
	m_pButtons->SetPos(new XMFLOAT2(205.0f, yStartPof - (yOffset * i++)), 3);

	m_pButtons->SetScale(new XMFLOAT2(1.0f, 1.0f), OPTSETALL);
	m_pButtons->CreateCollisionBox();

}

void PauseScene::Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_pPauseScreen->Render(pCommandList);
	m_pButtons->Render(pCommandList);

	for (int i = 0; i < m_nTextureUI; ++i) {
		m_pTextureUI[i]->Render(pCommandList);
	}
}


void GameOverScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Anitype = CLOSE_FIRST;

	vector<TextureDataForm> texutredata(1);
	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameOver.dds";

	m_pBlocks = make_unique<UIFullScreenShaders>();
	m_pBlocks->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);

	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameExit.dds";
	texutredata[0].m_MaxX = 2.0f;
	texutredata[0].m_MaxY = 1.0f;

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

void ClearScene::BuildScene(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList)
{
	m_Anitype = CLOSE_FIRST;

	vector<TextureDataForm> texutredata(1);
	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameOver.dds";

	m_pBlocks = make_unique<UIFullScreenShaders>();
	m_pBlocks->BuildObjects(pDevice, pCommandList, 1, &texutredata[0]);

	texutredata[0].m_texture = L"res\\MainSceneTexture\\GameExit.dds";
	texutredata[0].m_MaxX = 2.0f;
	texutredata[0].m_MaxY = 1.0f;

	m_pButtons = make_unique<UIButtonShaders>();
	m_pButtons->BuildObjects(pDevice, pCommandList, 1, &texutredata);
	m_pButtons->SetAlpha(0.0f, 0);
	m_pButtons->SetPosScreenRatio(XMFLOAT2(0.5f, 2.25 / 9.0f));
}
