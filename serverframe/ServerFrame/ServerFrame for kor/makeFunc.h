#pragma once

void Init_Data(vector<Player*> v) {};
int Init_Network(WSADATA wsa, HANDLE hcp);
int Create_Thread(HANDLE hcp, HANDLE aCceptThread, HANDLE hThread);
int Id_Send(vector<Player*> v, int &pCount, SOCKET sock, char* buf, int len, bool flag);