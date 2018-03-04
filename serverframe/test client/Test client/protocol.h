#pragma once
#pragma pack(1)

struct tPacket1 {
	float f1;
	float f2;
	float f3;
	float f4;
};

struct Player {
	int p_id;
	float p_x;
	float p_y;
	float p_z;
	int p_hp;
	char end;
};