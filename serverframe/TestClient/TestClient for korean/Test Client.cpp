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

	memset(buf, 0, sizeof(buf));

	Player my_p;
	Player *my_ptr;

	my_p.p_id = rand() % 4 + 1;
	my_p.p_x = 100.54;
	my_p.p_y = 120.12;
	my_p.p_z = 243.09;
	my_p.p_hp = 100;
	my_p.end = 'A';

	my_ptr = &my_p;

	*buf = (char)my_ptr;

	int len;

	while (1) {
		//���� ������
		//----��Ŷ �����͸� ĳ������ ���ۿ� ������� ����� ���� ������ �� ���۸� �����ؼ� ����
		//    ���������� ���� ��Ŷ�� ĳ������ ���ۿ� �ű��� ���� Ÿ�Կ� �°Բ� �о�鿩�� ������ ����




		////memset(&buf, 0, sizeof(buf));
		////���� �ʱ�ȭ�� �����ָ� ������ ���� �Ϳ� ���� memset���� ��� 0���� �ʱ�ȭ
		//
		////buf = 

		////sprintf_s(buf, "%5.2lf", my_p);

		////buf = (char *)p;

		//printf("\n[���� ������] : %f",buf[0]);



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
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
		printf("���� ������ : id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c",
			my_ptr->p_id, my_ptr->p_x, my_ptr->p_y, my_ptr->p_z, my_ptr->p_hp, my_ptr->end);
		count = 0;

		retval = recvn(sock, (char*)buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		buf[retval] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ������] %f\n", buf[0]);

		count++;
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}