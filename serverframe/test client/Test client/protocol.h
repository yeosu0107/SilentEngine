#pragma once

struct Pos {
	float x;
	float y;
	float z;
};

struct Player {
	int id;
	Pos p;
};

struct Packet {
	int id;
	char type;
};