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
	unique_ptr<UIFullScreenShaders> m_pBlocks; // �¿쿡�� �������� ����

	const float m_TotalAnimationTime = 4.0f;		// ��ü �ִϸ��̼� �ð�
	const float m_BlockMaxMovePos = 0.25f;			// �� �ִ� ��ġ
	const float m_BlockStartMovePos = -0.5f;		// �� ���� ��ġ
	const float m_AnimationRatio = 0.5f;			// �� �ִϸ��̼� �ð� ���� 
	const float m_ReOpenRatio = (m_BlockMaxMovePos - m_BlockStartMovePos) / 20.0f;

	float m_ProgessTime = 0.0f;

	Animation_TYPE	m_Anitype;
};

