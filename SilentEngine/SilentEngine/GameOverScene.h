#pragma once
#include "UIShaders.h"
#include "Timer.h"

class GameOverScene
{
	enum Animation_TYPE { CLOSE_FIRST, RE_OPEN, CLOSE_SECOND };
public:
	GameOverScene();
	~GameOverScene();

public:
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual bool Update(const Timer& gt);

protected:
	unique_ptr<UIFullScreenShaders> m_pBlocks; // 좌우에서 닫혀오는 블럭들

	const float m_TotalAnimationTime = 4.0f;		// 전체 애니메이션 시간
	const float m_BlockMaxMovePos = 0.25f;			// 블럭 최대 위치
	const float m_BlockStartMovePos = -0.5f;		// 블럭 시작 위치
	const float m_AnimationRatio = 0.5f;			// 블럭 애니메이션 시간 비율 
	const float m_ReOpenRatio = (m_BlockMaxMovePos - m_BlockStartMovePos) / 20.0f;

	float m_ProgessTime = 0.0f;

	Animation_TYPE	m_Anitype;
};

