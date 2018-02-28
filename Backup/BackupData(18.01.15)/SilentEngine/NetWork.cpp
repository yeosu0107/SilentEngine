#include "stdafx.h"
#include "NetWork.h"


NetWork::NetWork()
{
}


NetWork::~NetWork()
{
}

DWORD NetWork::CommunicationServer(void * arg)
{
	NetWork* pNetWork = (NetWork*)arg;
	SOCKET	 sock = pNetWork->m_sockServer;
	int retval;

	while (pNetWork->m_IsConnected) {
		// 30프레임으로 동작
		pNetWork->m_Timer->Tick(60.0f);
		retval = send(sock, pNetWork->m_sSendBuf, pNetWork->MAX_BUF, 0);
		if (retval == SOCKET_ERROR) {
			pNetWork->m_IsConnected = false;
			return 0;
		}

		retval = recv(sock, pNetWork->m_sRecvBuf, pNetWork->MAX_BUF, 0);
		if (retval == SOCKET_ERROR) {
			pNetWork->m_IsConnected = false;
			return 0;
		}
		
	}
	LeaveCriticalSection(&pNetWork->m_WaitForEndThread);
	return 0;
}

int NetWork::InitNetwork()
{
	InitializeCriticalSection(&m_SendRecvDataCriticalSection);
	InitializeCriticalSection(&m_ConnectDisconnectCriticalSection);
	InitializeCriticalSection(&m_WaitForEndThread);

	m_sSendBuf = new char[MAX_BUF];
	m_sRecvBuf = new char[MAX_BUF];
	m_Timer = std::make_unique<CGameTimer>();

	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return 1;

}

void NetWork::ConnectServer(char * pIPAddress, USHORT nPortnumber)
{
	EnterCriticalSection(&m_ConnectDisconnectCriticalSection);
		SOCKADDR_IN clientAddr;
		int retval = SOCKET_ERROR;
		
		m_sockServer = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sockServer == INVALID_SOCKET)
			return ;

		clientAddr.sin_family = AF_INET;
		clientAddr.sin_addr.s_addr = inet_addr(pIPAddress);
		clientAddr.sin_port = htons(nPortnumber);

		retval = connect(m_sockServer, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
		m_hThreadHandle = CreateThread(NULL, 0, CommunicationServer, this, 0, nullptr);
		m_IsConnected = true;
	LeaveCriticalSection(&m_ConnectDisconnectCriticalSection);
}

void NetWork::SendData(char * pBuf)
{
	memcpy(m_sSendBuf, pBuf, MAX_BUF);
}

void NetWork::GetData(char * pBuf)
{
	memcpy(pBuf, m_sRecvBuf, MAX_BUF);
}

void NetWork::Disconnect()
{
	EnterCriticalSection(&m_ConnectDisconnectCriticalSection);
		EnterCriticalSection(&m_WaitForEndThread);
			if (!m_IsConnected) return;
			m_IsConnected = false;
	LeaveCriticalSection(&m_ConnectDisconnectCriticalSection);
}
