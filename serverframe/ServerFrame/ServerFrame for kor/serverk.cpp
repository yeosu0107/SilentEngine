#include "stdafx.h"



Player guest;
Player* p_guest;

char SendBuf[BUFSIZE];

int id_count = 1;
vector <Player*> pLayer;

//
WSADATA wsa;
HANDLE hThread, aCceptThread;
HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

int main(int argc, char *argv[])
{
	Player* p;
	for (int i = 0; i < 4; ++i) {
		p->p_id = i + 1;
		p->p_x = 10;
		p->p_y = 0;
		p->p_z = 10;
		p->p_hp = 100;
		p->end = 'P';

		pLayer.push_back(p);
	}

	Init_Network(wsa,hcp);
	
	Create_Thread(hcp, hThread, aCceptThread);

	while(1){}	

	WSACleanup();

	return 0;
}

DWORD WINAPI AcceptThread(LPVOID arg) {
	
	HANDLE hcp = (HANDLE)arg;
	
	int retval;

	Listen_sock listen_sock;
	
	retval = listen_sock.Listen_bind();
	if (retval == 0) {
		cout << "bind()" << endl;
	}

	retval = listen_sock.set_Liten();
	if (retval == 0) {
		cout << "listen()" << endl;
	}

	//SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while (1) {
		DWORD cbTransferred;
		SOCKET client_sock;
		SOCKETINFO *ptr = new SOCKETINFO();

		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock.get_Socket(), (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		printf("[TCP 서버] 클라이언트 접속 : IP 주소 - %s, 포트번호 - %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		///-----------------------------
		Id_Send(pLayer, id_count, client_sock, SendBuf, sizeof(SendBuf), 0);
		//------------------------------

		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);
		recvbytes = 0;

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

		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (a = WSAGetLastError() != ERROR_IO_PENDING) {
				printf("%d\n\n", a);
				err_display("WSARecv()");
			}
			continue;
		}

	}

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
			ptr->recvbytes = cbTransferred;
			ptr->sendbytes = 0;
			
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