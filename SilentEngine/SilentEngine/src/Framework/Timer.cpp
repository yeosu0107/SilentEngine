#include "stdafx.h"
#include "Timer.h"


Timer::Timer() :
	m_dSecondsPerCount(0.0), 
	m_dDeltaTime(-1.0), 
	m_nBaseTime(0),
	m_nPausedTime(0), 
	m_nPrevTime(0), 
	m_nCurrTime(0), 
	m_bStopped(false)
{
	INT64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_dSecondsPerCount = 1.0 / (double)countsPerSec;
}

float Timer::TotalTime() const
{
	if (m_bStopped)
		return (float)(((m_nStopTime - m_nPausedTime) - m_nBaseTime) * m_dSecondsPerCount);

	else
		return (float)(((m_nCurrTime - m_nPausedTime) - m_nBaseTime) * m_dSecondsPerCount);
}

float Timer::DeltaTime() const
{
	return (float)m_dDeltaTime;
}

void Timer::Reset()
{
	INT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_nBaseTime = currTime;
	m_nPrevTime = currTime;
	m_nStopTime = 0;
	m_bStopped	= false;
}

void Timer::Start()
{
	INT64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_bStopped)
	{
		m_nPausedTime += (startTime - m_nStopTime);

		m_nPrevTime = startTime;
		m_nStopTime = 0;
		m_bStopped  = false;
	}
}

void Timer::Stop()
{
	if (!m_bStopped)
	{
		INT64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_nStopTime = currTime;
		m_bStopped	= true;
	}
}

void Timer::Tick()
{
	if (m_bStopped) {
		m_dDeltaTime = 0.0f;
		return;
	}

	INT64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_nCurrTime		= currTime;
	m_dDeltaTime	= (m_nCurrTime - m_nPrevTime) * m_dSecondsPerCount;
	m_nPrevTime		= m_nCurrTime;

	if (m_dDeltaTime < 0.0)
		m_dDeltaTime = 0.0;
}
