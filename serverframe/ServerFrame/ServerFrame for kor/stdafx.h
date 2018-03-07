// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
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