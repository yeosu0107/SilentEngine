#pragma once

#include <queue>

const int MAX_EVERAGE_FRAME = 60;

class Timer
{
private:
	double		m_dPreframeTime;
	double		m_dCurframeTime;

	int			m_iCurrentframeRate;
	int			m_iCountframeRate;

	float		m_fElapsedTime;
	float		m_ffps;
	float		m_fTotalframeRate;
	float		m_fAverageframeTime;
	float		m_fTimeScale;

	std::queue<float>  m_pEverageFrame;

public:
	Timer();
	Timer(float fps);
	~Timer();

public:
	void Update(float fps);

	std::string GetFrameTime();

	float Tick() const { return m_fElapsedTime; }
	void Tock() { m_dPreframeTime = m_dCurframeTime; }
};
