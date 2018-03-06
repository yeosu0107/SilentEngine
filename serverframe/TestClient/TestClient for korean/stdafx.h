#pragma once
#pragma comment(lib, "ws2_32")

#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include "protocol.h"

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

void err_quit(char *msg);
void err_display(char *msg);

int recvn(SOCKET s, char *buf, int len, int flags);
