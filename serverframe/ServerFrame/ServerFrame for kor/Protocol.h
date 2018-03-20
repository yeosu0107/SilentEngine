#pragma once
//#pragma pack(1)

#define CHARACTER 100
#define LEFT 201
#define UP 202
#define RIGHT 203
#define DOWN 204

class Player {
public:
	char pack_type;
	int p_id;
	float p_x;
	float p_y;
	float p_z;
	int p_hp;
	char end;
};

struct Move {
	char pack_type;
	int p_id;
};