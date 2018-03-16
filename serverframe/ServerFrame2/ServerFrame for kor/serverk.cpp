#include "stdafx.h"

Player guest;
Player* p_guest;

Move mPacket;
Move *p_mPacket;

Player Guest[4];

int main(int argc, char *argv[])
{
	for (int i = 0; i < 4; ++i) {
		Guest[i].pack_type = CHARACTER;
		Guest[i].p_id = i + 1;
		Guest[i].p_x = 10;
		Guest[i].p_y = 0;
		Guest[i].p_z = 10;
		Guest[i].p_hp = 100;
		Guest[i].end = 'P';
	}

	p_guest = &guest;

	int id_count = 1;
	char SendBuf[BUFSIZE];

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

		printf("[TCP 서버] 클라이언트 접속 : IP 주소 - %s, 포트번호 - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		///-----------------------------
		for (int i = 0; i < 4; ++i) {
			if (id_count == Guest[i].p_id) {
				memcpy(SendBuf, &Guest[i], sizeof(Guest[i]));
				retval = send(client_sock, SendBuf, sizeof(SendBuf), 0);
			}
			else {
				id_count++;
				break;
			}
		}
		//------------------------------

		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);
		recvbytes = 0;
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

		//retval = recv(client_sock, ptr->buf, 21, 0);

		//retval = recv(client_sock, (char*)p_guest, 21, 0);

		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (a = WSAGetLastError() != ERROR_IO_PENDING) {
				printf("%d\n\n", a);
				err_display("WSARecv()");
			}
			continue;
		}

		//memcpy(p_guest, ptr->buf, sizeof(p_guest));

		//guest.p_hp = p_guest->p_hp;
		//guest.p_id = p_guest->p_id;
		//guest.p_x = p_guest->p_x;
		//guest.p_y = p_guest->p_y;
		//guest.p_z = p_guest->p_z;
		//guest.end = p_guest->end;

		//// 확인용
		//printf("[TCP/%s:%d] id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),
		//	guest.p_id, guest.p_x, guest.p_y, guest.p_z, guest.p_hp, guest.end);
		////-----------

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

		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (PULONG_PTR)&client_sock, (LPOVERLAPPED *)&ptr, INFINITE);

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
			printf("[TCP서버] 클라이언트 종료 : IP주소 - %s, 포트번호 - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}

		if (ptr->recvbytes == 0) {
			//	ptr->recvbytes = cbTransferred;
			//	ptr->sendbytes = 0;
			if (ptr->buf[0] == CHARACTER) {
				memcpy(p_guest, ptr->buf, sizeof(p_guest));

				guest.p_hp = p_guest->p_hp;
				guest.p_id = p_guest->p_id;
				guest.p_x = p_guest->p_x;
				guest.p_y = p_guest->p_y;
				guest.p_z = p_guest->p_z;
				guest.end = p_guest->end;

				// 확인용
				printf("[TCP/%s:%d] id : %d, pos : ( %f, %f, %f ), hp : %d, end : %c\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),
					guest.p_id, guest.p_x, guest.p_y, guest.p_z, guest.p_hp, guest.end);
			}
			else {
				memcpy(p_mPacket, ptr->buf, sizeof(p_mPacket));
				switch (p_mPacket->pack_type) {
				case LEFT:
					Guest[p_mPacket->p_id].p_x -= 1;

				case RIGHT:
					Guest[p_mPacket->p_id].p_x += 1;

				case UP:
					Guest[p_mPacket->p_id].p_y -= 1;

				case DOWN:
					Guest[p_mPacket->p_id].p_x += 1;
				}
			}
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