#pragma once
#include "GameObjects.h"

class UIObject
{
public:
	UIObject();
	~UIObject();

public:

protected:
	XMFLOAT3 m_xmf3Pos;

	bool m_bEnabled;
	
	float m_fWidth;
	float m_fHeight;

	float m_fXScale;
	float m_fYScale;

	
};

