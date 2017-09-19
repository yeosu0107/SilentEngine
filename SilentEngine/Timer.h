#pragma once

const unsigned long MAX_SAMPLE_COUNT = 50;
// 50회의 프레임 처리시간을 누적하여 평균한다

class CGameTimer
{
private:
	bool m_bHardwareHasPerformanceCounter; //컴퓨터가 퍼포먼스카운터 가지고있는지?
	float m_fTimeScale;		//스케일 카운터 양
	float m_fTimeElapsed;	//마지막 프레임 이후 지난 시간

	__int64 m_nCurrentTime; //현재시간   
	__int64 m_nLastTime;	//마지막 프레임 시간

	__int64 m_nBaseTime;
	__int64 m_nPausedTime;
	__int64 m_nStopTime;

	__int64 m_nPerformanceFrequency; //컴퓨터의 퍼포먼스프리퀀시

	float m_fFrameTime[MAX_SAMPLE_COUNT];   //프레임 시간 누적위한배열
	unsigned long m_nSampleCount;					//누적 프레임 횟수

	unsigned long m_nCurrentFrameRate;	//현재 프레임레이트
	unsigned long m_nFramesPerSecond;	//초당 프레임수
	float m_fFPSTimeElapsed;			//프레임레이트 계산 소요 시간

	bool m_bStopped;

public:
	CGameTimer();
	virtual ~CGameTimer();

	void Tick(float fLockFPS = 0.0f);
	void Start();
	void Stop();
	void Reset();
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();
	float GetTotalTime();
};
