//***************************************************************************************
// Timer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include <windows.h>
#include "stdafx.h"
#include "Timer.h"



Timer::Timer()
: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), 
  mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


float Timer::TotalTime()const
{
	
	if( mStopped )
	{
		return (float)(((mStopTime - mPausedTime)-mBaseTime)*mSecondsPerCount);
	}
	
	else
	{
		return (float)(((mCurrTime-mPausedTime)-mBaseTime)*mSecondsPerCount);
	}
}

float Timer::DeltaTime()const
{
	return (float)mDeltaTime;
}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped  = false;
}

void Timer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if( mStopped )
	{
		mPausedTime += (startTime - mStopTime);	

		mPrevTime = startTime;
		mStopTime = 0;
		mStopped  = false;
	}
}

void Timer::Stop()
{
	if( !mStopped )
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		mStopped  = true;
	}
}

void Timer::Tick()
{
	if( mStopped )
	{
		mDeltaTime = 0.0;
		return;
	}
	__int64 currTime;

	LARGE_INTEGER frameinfo;
	QueryPerformanceFrequency(&frameinfo);
	const float perframe = (float)frameinfo.QuadPart / 60.0f;

	while (true) {
		
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mCurrTime = currTime;

		// Time difference between this frame and the previous.
		mDeltaTime = (mCurrTime - mPrevTime);
		if (mDeltaTime > perframe)
			break;
	}

	mPrevTime = mCurrTime;
	
	if(mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

