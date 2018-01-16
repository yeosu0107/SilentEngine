#pragma once

class Timer
{
public:
	Timer();

	float	TotalTime() const;
	float	DeltaTime() const;

	void	Reset();
	void	Start();
	void	Stop();
	void	Tick();

private:
	double	m_dSecondsPerCount;
	double	m_dDeltaTime;

	INT64	m_nBaseTime;
	INT64	m_nPausedTime;
	INT64	m_nStopTime;
	INT64	m_nPrevTime;
	INT64	m_nCurrTime;

	bool	m_bStopped;
};

