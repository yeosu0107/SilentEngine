#pragma once

enum GameMode {
	Mode_Single = 0,
	Mode_Multi = 1
};

const int MAX_USER = 4; //�ڱ� �ڽ��� ���� 4��

class NetWorkManager
{
private:
	int num_User;
public:
	NetWorkManager() : num_User(0) { }
	~NetWorkManager() { }

	int GetUser() const { return num_User; }
};