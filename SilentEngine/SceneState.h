#pragma once
#include "UIShaders.h"
#include "Timer.h"

const int m_nTextureUI = 3;

class virtualScene {
public:
	virtualScene() = default;
	~virtualScene() = default;

public:
	virtual int CollisionToMouseClick(POINT* mousePos);
	virtual void ColiisionToMouseMove(POINT* mousePos);

	virtual void SetPoint(POINT* pPoint) { m_pButtons->SetPoint(pPoint); }

	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {};
	virtual void Render(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {};
	virtual void Reset() {};
	virtual bool Update(const Timer& gt) { return false;};

protected:
	enum Animation_TYPE { CLOSE_FIRST, RE_OPEN, CLOSE_SECOND, APPEAR_BOTTON, Animate_None };

	unique_ptr<UIButtonShaders> m_pButtons;
};

class PauseScene : public virtualScene
{
public:
	PauseScene() = default;
	~PauseScene() = default;

public:
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	
protected:

	unique_ptr<UIShaders> m_pTextureUI[m_nTextureUI];
	unique_ptr<TextureToFullScreen> m_pPauseScreen = nullptr;
};

class GameOverScene : public virtualScene
{
	
public:
	GameOverScene() = default;
	~GameOverScene() = default;

public:
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Reset();
	virtual bool Update(const Timer& gt);

protected:
	unique_ptr<UIFullScreenShaders> m_pBlocks; // �¿쿡�� �������� ����

	const float m_TotalAnimationTime = 1.0f;		// ��ü �ִϸ��̼� �ð�
	const float m_BlockMaxMovePos = 0.25f;			// �� �ִ� ��ġ
	const float m_BlockStartMovePos = -0.25f;		// �� ���� ��ġ
	const float m_MidPos = (m_BlockStartMovePos + m_BlockMaxMovePos) / 2.0f;
	const float m_ReOpenRatio = -0.05f;

	float m_ProgessTime = 0.0f;

	Animation_TYPE	m_Anitype;
};

class ClearScene : public GameOverScene
{
public:
	ClearScene() = default;
	~ClearScene() = default;

public:
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
};