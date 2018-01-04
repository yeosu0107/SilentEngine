// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once
#pragma comment(lib, "ws2_32")

#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

void err_quit(char *msg);
void err_display(char *msg);

int recvn(SOCKET s, char *buf, int len, int flags);