#include "stdafx.h"

char id[4];
char p_x[4];
char p_y[4];
char p_z[4];
char p_hp[4];

int main(int argc, char *argv[])
{
	Player guest;

	int retval;

	printf("char : %d, int : %d, float : %d\n", sizeof(char), sizeof(int), sizeof(float));

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == 0) {
		return 1;
	}

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i) {
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL) {
			return 1;
		}
		CloseHandle(hThread);
	}

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		err_quit("socket()");
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
	}

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while (1) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		printf("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� - %s, ��Ʈ��ȣ - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));



		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);
		recvbytes = 21;
		SOCKETINFO *ptr = new SOCKETINFO();
		if (ptr == NULL) {
			break;
		}
		int a = 0;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;
		ZeroMemory(ptr->buf, sizeof(ptr->buf));
		ZeroMemory(ptr->wsabuf.buf, sizeof(ptr->buf));
		flags = 0;

		retval = recv(client_sock, ptr->buf, 21, 0);

		//retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (a = WSAGetLastError() != ERROR_IO_PENDING) {
				printf("%d\n\n", a);
				err_display("WSARecv()");
			}
			continue;
		}

		// Ȯ�ο�
		for (int i = 0; i < 4; ++i) {
			id[0] = ptr->buf[0];
			p_x[i] = ptr->buf[i + 1];
			p_y[i] = ptr->buf[i + 5];
			p_z[i] = ptr->buf[i + 9];
			p_hp[i] = ptr->buf[i + 13];
		}

		guest.p_id = ptr->buf[0];
		guest.p_hp = ptr->buf[16];

		/*guest.p_id = id[0];
		guest.p_hp = (int)p_hp;
		guest.p_x = (float)p_x;
		guest.p_y = (float)p_y;
		guest.p_z = (float)p_z;*/

		//sprintf((char*)&guest.p_id, "%d", id, sizeof(id);
		//sprintf((char*)&guest.p_hp, "%d", p_hp, sizeof(int));
		sprintf((char*)&guest.p_x, "%f", p_x, sizeof(p_x));
		sprintf((char*)&guest.p_y, "%f", p_y, sizeof(p_y));
		sprintf((char*)&guest.p_z, "%f", p_z, sizeof(p_z));

		//guest.p_id = ptr->buf[0];
		//guest.p_x = ptr->buf[4];
		//guest.p_y = ptr->buf[8];
		//guest.p_z = ptr->buf[12];
		//guest.p_hp = ptr->buf[16];


		printf("[TCP/%s:%d] id : %d, pos : ( %f, %f, %f ), hp : %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),
			guest.p_id, guest.p_x, guest.p_y, guest.p_z, guest.p_hp);
		//-----------

	}

	WSACleanup();

	return 0;
}

DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;

	while (1) {
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr;

		//Player guest;

		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&client_sock, (LPOVERLAPPED *)&ptr, INFINITE);

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);

		if (retval == 0 || cbTransferred == 0) {
			if (retval == 0) {
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);
				err_display("WSAGetOverlappedresult()");
			}
			closesocket(ptr->sock);
			printf("[TCP����] Ŭ���̾�Ʈ ���� : IP�ּ� - %s, ��Ʈ��ȣ - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}

		if (ptr->recvbytes == 0) {
			//	ptr->recvbytes = cbTransferred;
			//	ptr->sendbytes = 0;
			//	ptr->buf[ptr->recvbytes] = '\0';

			//	for (int i = 0; i < 4; ++i) {
			//		//if (i < 4) {
			//			id[i] = ptr->buf[i];
			//			p_hp[i] = ptr->buf[i + 16];
			//		//}
			//		p_x[i] = ptr->buf[i+4];
			//		p_y[i] = ptr->buf[i + 8];
			//		p_z[i] = ptr->buf[i + 12];
			//	}

			//	sprintf((char*)&guest.p_id, "%d", id, sizeof(int));
			//	sprintf((char*)&guest.p_hp, "%d", p_hp, sizeof(int));
			//	sprintf((char*)&guest.p_x, "%f", p_x, sizeof(float));
			//	sprintf((char*)&guest.p_y, "%f", p_y, sizeof(float));
			//	sprintf((char*)&guest.p_z, "%f", p_z, sizeof(float));

			//sprintf_s(guest.id, "%d", ptr->buf,sizeof(int));

			//ptr->buf[0] = 

			/*printf("[TCP/%s:%d] id : %d, pos : ( %f, %f, %f ), hp : %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),
			guest.p_id, guest.p_x, guest.p_y, guest.p_z, guest.p_hp);*/
		}
		else {
			ptr->sendbytes += cbTransferred;
		}

		if (ptr->recvbytes > ptr->sendbytes) {
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
			ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

			DWORD sendbytes;
			retval = WSASend(ptr->sock, &ptr->wsabuf, 1, &sendbytes, 0, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					err_display("WSASend()");
				}
				continue;
			}
		}
		else {
			ptr->recvbytes = 0;

			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = ptr->buf;
			ptr->wsabuf.len = BUFSIZE;

			DWORD recvbytes;
			DWORD flags = 0;
			retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					err_display("WSARecv()");
				}
				continue;
			}
		}
	}
	return 0;
}