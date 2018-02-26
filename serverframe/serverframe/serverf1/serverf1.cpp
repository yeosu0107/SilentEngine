#include "stdafx.h"

Player client;


int main(int argc, char *argv[])
{
	// 클라이언트 초기정보
	/*for (int i = 0; i < 4; ++i) {
		client[i].id = i + 1;
		client[i].p.x = 0;
		client[i].p.y = 0;
		client[i].p.z = 0;
	}*/
	client.id = 1;
	client.p.x = 0;
	client.p.y = 0;
	client.p.z = 0;

	int retval;

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

		//for (int i = 0; i < 4; ++i) {
		//retval = send(client_sock, (char *)client, sizeof(client), 0);
		//}

		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);

		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) {
			break;
		}
		
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = (char*)ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		flags = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				err_display("WSARecv()");
			}
			continue;
		}
	}

	WSACleanup();

    return 0;
}

DWORD WINAPI WorkerThread(LPVOID arg)
{
	int retval;
	HANDLE hcp = (HANDLE)arg;

	Packet p;
	p.id = -1;
	p.type = '\0';

	while (1) {
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr;
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
			printf("[TCP서버] 클라이언트 종료 : IP주소 - %s, 포트번호 - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			delete ptr;
			continue;
		}
		
		printf("%d :: %c\n", p.id, p.type);
		

		p = ptr->buf[0];

		/*switch (p.type) {
		case 'w' :
			client.p.z++;
			retval = send(client_sock, (char *)client, sizeof(client), 0);
		case 's' :
			client.p.z--;
			retval = send(client_sock, (char *)client, sizeof(client), 0);
		case 'a' :
			client.p.x--;
			retval = send(client_sock, (char *)client, sizeof(client), 0);
		case 'd' :
			client.p.x++;
			retval = send(client_sock, (char *)client, sizeof(client), 0);
		}*/

		//printf("%d : %c - %f, %f, %f\n", p.type ,client[0].id, client[0].p.x, client[0].p.y, client[0].p.z);
		
		if (ptr->recvbytes == 0) {
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			//ptr->buf[ptr->recvbytes] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), ptr->buf);
		}
		else {
			ptr->sendbytes += cbTransferred;
		}
		
		//Send 부분
		if (ptr->recvbytes > ptr->sendbytes) {
			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = (char*)ptr->buf + ptr->sendbytes;
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

		//Recv 부분
		else {
			ptr->recvbytes = 0;

			ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
			ptr->wsabuf.buf = (char*)ptr->buf;
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