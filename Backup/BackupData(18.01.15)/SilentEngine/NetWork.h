#pragma once
#pragma warning (disable : 4819)
#pragma warning (disable : 4996)

#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <process.h>
#include <memory>
#include "Code\FrameWork\Timer.h"

class NetWork
{
public:
	NetWork();
	~NetWork();

public:
	static DWORD WINAPI CommunicationServer(void* arg);	 // 통신을 위한 쓰레드 함수
	
	int InitNetwork();
	void ConnectServer(char* pIPAddress, USHORT nPortnumber);
	void SendData(char* pBuf);
	void GetData(char* pBuf);

	void Disconnect();
		
private:
	bool	m_IsConnected = false;
	const UINT MAX_BUF = 32;
	
	char*	m_sSendBuf = nullptr;
	char*	m_sRecvBuf = nullptr;

	USHORT	m_nPort;

	SOCKET	m_sockServer;
	WSADATA	m_wsa;

	std::unique_ptr<CGameTimer> m_Timer;

	CRITICAL_SECTION m_SendRecvDataCriticalSection;
	CRITICAL_SECTION m_ConnectDisconnectCriticalSection;
	CRITICAL_SECTION m_WaitForEndThread;

	HANDLE	m_hThreadHandle;
};

