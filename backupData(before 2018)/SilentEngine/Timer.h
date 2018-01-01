#pragma once

const unsigned long MAX_SAMPLE_COUNT = 50;
// 50ȸ�� ������ ó���ð��� �����Ͽ� ����Ѵ�

class CGameTimer
{
private:
	bool m_bHardwareHasPerformanceCounter; //��ǻ�Ͱ� �����ս�ī���� �������ִ���?
	float m_fTimeScale;		//������ ī���� ��
	float m_fTimeElapsed;	//������ ������ ���� ���� �ð�

	__int64 m_nCurrentTime; //����ð�   
	__int64 m_nLastTime;	//������ ������ �ð�

	__int64 m_nBaseTime;
	__int64 m_nPausedTime;
	__int64 m_nStopTime;

	__int64 m_nPerformanceFrequency; //��ǻ���� �����ս���������

	float m_fFrameTime[MAX_SAMPLE_COUNT];   //������ �ð� �������ѹ迭
	unsigned long m_nSampleCount;					//���� ������ Ƚ��

	unsigned long m_nCurrentFrameRate;	//���� �����ӷ���Ʈ
	unsigned long m_nFramesPerSecond;	//�ʴ� �����Ӽ�
	float m_fFPSTimeElapsed;			//�����ӷ���Ʈ ��� �ҿ� �ð�

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
