// Testclient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"


int main()
{
	int retval;
	int count = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		err_quit("sock");
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("connect");
	}

	char buf[BUFSIZE + 1];

	Player my_p;
	Player *my_ptr;

	my_p.p_id = rand()%4+1;
	my_p.p_x  = 0;
	my_p.p_y  = 0;
	my_p.p_z  = 0;
	my_p.p_hp = 100;
	my_p.end = '\0';

	my_ptr = &my_p;


	int len;

	while (1) {
		//보낼 데이터
		//----패킷 데이터를 캐릭터형 버퍼에 순서대로 사이즈에 집어 넣은후 그 버퍼를 연결해서 전송
		//    서버에서도 받은 패킷을 캐릭터형 버퍼에 옮긴후 각각 타입에 맞게끔 읽어들여서 데이터 편집

		


		////memset(&buf, 0, sizeof(buf));
		////가끔 초기화를 안해주면 에러가 나는 것에 의해 memset으로 모드 0으로 초기화
		//
		////buf = 

		////sprintf_s(buf, "%5.2lf", my_p);

		////buf = (char *)p;

		//printf("\n[보낼 데이터] : %f",buf[0]);
		


		//if (fgets(buf, BUFSIZE + 1, stdin) == NULL) {
		//	break;
		//}

		//len = strlen(buf);
		//if (buf[len - 1] == '\n') {
		//	buf[len - 1] = '\0';
		//}
		//if (strlen(buf) == 0) {
		//	break;
		//}



		//retval = send(sock, (char*)my_p, strlen((char*)buf), 0);
		retval = send(sock, (char*)my_ptr, sizeof(Player), 0);

			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
			count = 0;
		
		retval = recvn(sock, (char*)buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터] %f\n", buf[0]);

		count++;
	}

	closesocket(sock);
	WSACleanup();

    return 0;
}

