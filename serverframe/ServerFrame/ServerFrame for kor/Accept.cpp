#include "stdafx.h"

Listen_sock::Listen_sock() {
	socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		err_quit("socket()");
	}
}

int Listen_sock::set_Liten() {
	int retval;

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
	}
	return retval;
};

int Listen_sock::Listen_bind() {
	int retval;

	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	return retval;
};

SOCKET Listen_sock::get_Socket() {
	return listen_sock;
}