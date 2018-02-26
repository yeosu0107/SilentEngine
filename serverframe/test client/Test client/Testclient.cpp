// Testclient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#define BUFSIZE 50

int main()
{
	Player pLayer;
	char key;

	Player* ptr = &pLayer;
	
	Packet p;

	//p.id = -1;
	//p.type = '\0';

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

	retval = recv(sock, (char*)ptr, sizeof(Player), 0);

	printf("%d : %f, %f, %f\n", pLayer.id, pLayer.p.x, pLayer.p.y, pLayer.p.z);


	while (1) {

		memset(&key, '\0', sizeof(char));
		//fflush();
		scanf("%c", &key);
		//fgets(&key, 1, stdin);
		switch (key) {
		case 'w' : 
			p.id = pLayer.id;
			p.type = 'w';

			retval = send(sock, (char*)&p, sizeof(p), 0);
			//pLayer[0].p.z += 1;
			//key = '\0';

		case 's' : 
			p.id = pLayer.id;
			p.type = 's';

			retval = send(sock, (char*)&p, sizeof(p), 0);
			//pLayer[0].p.z -= 1;
			//key = '\0';

		case 'a' : 
			p.id = pLayer.id;
			p.type = 'a';

			retval = send(sock, (char*)&p, sizeof(p), 0);
			//pLayer[0].p.x -= 1;
			//key = '\0';

		case 'd' : 
			p.id = pLayer.id;
			p.type = 'd';

			retval = send(sock, (char*)&p, sizeof(p), 0);
			//pLayer[0].p.x += 1;
			//key = '\0';
		}

		retval = recv(sock, (char*)ptr, sizeof(Player), 0);

		printf("%d : %f, %f, %f\n", pLayer.id, pLayer.p.x, pLayer.p.y, pLayer.p.z);
		//fgets(buf, BUFSIZE + 1, stdin);




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

		//retval = send(sock, buf, strlen(buf), 0);
		/*if (count == 1000) {
			retval = send(sock, (char*)buf, sizeof(Packet), 0);

			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
			count = 0;
		}*/
		/*retval = recvn(sock, (char)p, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터] %f\n", buf[0]);

		count++;*/
	}

	closesocket(sock);
	WSACleanup();

    return 0;
}

