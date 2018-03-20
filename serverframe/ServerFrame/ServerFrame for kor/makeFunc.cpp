#include "stdafx.h"


int Init_Network(WSADATA wsa, HANDLE hcp) {
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}
		
	if (hcp == 0) {
		return 1;
	}
}

int Create_Thread(HANDLE hcp, HANDLE aCceptThread, HANDLE hThread) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	aCceptThread = CreateThread(NULL, 0, AcceptThread, hcp, 0, NULL);

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i) {
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL) {
			return 1;
		}
		CloseHandle(hThread);
	}
}

int Id_Send(vector<Player*> v, int &pCount, SOCKET sock, char* buf, int len, bool flag) {
	int retval;
	for (int i = 0; i < 4; ++i) {
		if (pCount == v[i]->p_id) {
			memcpy(buf, v[i], sizeof(v[i]));
			retval = send(sock, buf, sizeof(buf), 0);
		}
		else {
			pCount++;
			break;
		}
	}

	return retval;
}