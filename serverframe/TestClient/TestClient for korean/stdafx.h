#pragma once
#pragma comment(lib, "ws2_32")

#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include "protocol.h"

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

void err_quit(char *msg);
void err_display(char *msg);

int recvn(SOCKET s, char *buf, int len, int flags);
