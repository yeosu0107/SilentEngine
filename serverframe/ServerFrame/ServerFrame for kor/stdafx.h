// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#pragma once
#pragma comment(lib, "ws2_32")

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <WS2tcpip.h>
#include "Protocol.h"

#define SERVERPORT 8000
#define BUFSIZE 512

DWORD WINAPI AcceptThread(LPVOID arg);
DWORD WINAPI WorkerThread(LPVOID arg);

void err_quit(char *msg);
void err_display(char *msg);

struct SOCKETINFO {
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};