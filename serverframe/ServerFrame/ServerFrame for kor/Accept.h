#pragma once
#include "stdafx.h"

class Listen_sock {
protected:
	SOCKET listen_sock;
	SOCKADDR_IN serveraddr;

public:
	Listen_sock();
	~Listen_sock() {};

	virtual int set_Liten();
	virtual int Listen_bind();
	virtual SOCKET get_Socket();
};