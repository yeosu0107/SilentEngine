#include "stdafx.h"



int main()
{
	char key;

	int retval;

	char RecvBuf[BUFSIZE];
	char SendBuf[BUFSIZE];
	
	Player my_Caracter;
	Player pPacket;
	Move mPacket;

	memset(SendBuf, 0, sizeof(SendBuf));
	memset(RecvBuf, 0, sizeof(RecvBuf));

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

	retval = send(sock, SendBuf, sizeof(SendBuf), 0);

	memset(RecvBuf, 0, sizeof(RecvBuf));

	retval = recv(sock,RecvBuf,PACK_MAX_SIZE,0);

	memcpy(&my_Caracter,RecvBuf,sizeof(my_Caracter));


	printf("받은 내 캐릭터 : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
		my_Caracter.p_id, my_Caracter.p_x, my_Caracter.p_y, my_Caracter.p_z, my_Caracter.p_hp, my_Caracter.end);

	//char buf[BUFSIZE + 1];

	/*Player my_p;
	Player *my_ptr;

	my_ptr = &my_p;*/

	//*buf = (char)my_ptr;

	int len;

	while (1) {
		//보낼 데이터
		//----패킷 데이터를 캐릭터형 버퍼에 순서대로 사이즈에 집어 넣은후 그 버퍼를 연결해서 전송
		//    서버에서도 받은 패킷을 캐릭터형 버퍼에 옮긴후 각각 타입에 맞게끔 읽어들여서 데이터 편집

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
		
		//retval = recv(sock, RecvBuf, PACK_MAX_SIZE, 0);

		//memcpy(&my_Caracter, RecvBuf, sizeof(my_Caracter));

		//// 데이터 확인용
		//printf("받은 내 캐릭터 : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
		//	my_Caracter.p_id, my_Caracter.p_x, my_Caracter.p_y, my_Caracter.p_z, my_Caracter.p_hp, my_Caracter.end);
		////--------------

		scanf_s("%c", &key);
		switch(key){
		case 'w': 
			mPacket.pack_type = UP;
			mPacket.p_id = my_Caracter.p_id;

			memcpy(SendBuf, &mPacket, sizeof(mPacket));
			//my_Caracter.p_y -= 1;

		case 's':
			mPacket.pack_type = DOWN;
			mPacket.p_id = my_Caracter.p_id;

			memcpy(SendBuf, &mPacket, sizeof(mPacket));
			//my_Caracter.p_y += 1;

		case 'a':
			//my_Caracter.p_x -= 1;
			mPacket.pack_type = LEFT;
			mPacket.p_id = my_Caracter.p_id;
			memcpy(SendBuf, &mPacket, sizeof(mPacket));

		case 'd':
			//my_Caracter.p_x += 1;
			mPacket.pack_type = RIGHT;
			mPacket.p_id = my_Caracter.p_id;
			memcpy(SendBuf, &mPacket, sizeof(mPacket));
		}
		
		
		
		//memcpy(SendBuf, &my_Caracter, sizeof(my_Caracter));


		retval = send(sock, SendBuf, sizeof(Player), 0);

		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
		printf("보낸 데이터 : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
			my_Caracter.p_id,my_Caracter.p_x,my_Caracter.p_y,my_Caracter.p_z,my_Caracter.p_hp,my_Caracter.end);
		//count = 0;

		retval = recvn(sock, RecvBuf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);

		memcpy(&my_Caracter, RecvBuf, sizeof(my_Caracter));

		// 데이터 확인용
		printf("받은 내 캐릭터 : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
			my_Caracter.p_id, my_Caracter.p_x, my_Caracter.p_y, my_Caracter.p_z, my_Caracter.p_hp, my_Caracter.end);
		//--------------

		//printf("[받은 데이터] %f\n", buf[0]);

		//count++;
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}