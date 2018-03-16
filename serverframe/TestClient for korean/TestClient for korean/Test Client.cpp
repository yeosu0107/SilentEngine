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


	printf("���� �� ĳ���� : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
		my_Caracter.p_id, my_Caracter.p_x, my_Caracter.p_y, my_Caracter.p_z, my_Caracter.p_hp, my_Caracter.end);

	//char buf[BUFSIZE + 1];

	/*Player my_p;
	Player *my_ptr;

	my_ptr = &my_p;*/

	//*buf = (char)my_ptr;

	int len;

	while (1) {
		//���� ������
		//----��Ŷ �����͸� ĳ������ ���ۿ� ������� ����� ���� ������ �� ���۸� �����ؼ� ����
		//    ���������� ���� ��Ŷ�� ĳ������ ���ۿ� �ű��� ���� Ÿ�Կ� �°Բ� �о�鿩�� ������ ����

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

		//// ������ Ȯ�ο�
		//printf("���� �� ĳ���� : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
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
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
		printf("���� ������ : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
			my_Caracter.p_id,my_Caracter.p_x,my_Caracter.p_y,my_Caracter.p_z,my_Caracter.p_hp,my_Caracter.end);
		//count = 0;

		retval = recvn(sock, RecvBuf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		//buf[retval] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);

		memcpy(&my_Caracter, RecvBuf, sizeof(my_Caracter));

		// ������ Ȯ�ο�
		printf("���� �� ĳ���� : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
			my_Caracter.p_id, my_Caracter.p_x, my_Caracter.p_y, my_Caracter.p_z, my_Caracter.p_hp, my_Caracter.end);
		//--------------

		//printf("[���� ������] %f\n", buf[0]);

		//count++;
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}