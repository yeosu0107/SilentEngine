#pragma once
#include "Shader.h"
#include "MapGenerator.h"
//const int MAX_X_AXIS = 9;
//const int MAX_Y_AXIS = 6;

class RoomShader;

class Stage {
private:
	Map currentMap;
	int stage_index;
	int room_index;
	int num_room;

	RoomShader** m_ppRoomShaders;
public:
	Stage(int x, int y, int index);
	~Stage();
};

class RoomShader : public CInstancingShader {
private:
	int num_x;
	int num_y;
	int num_tile;

	RoomShader* up;
	RoomShader* down;
	RoomShader* left;
	RoomShader* right;
public:
	RoomShader();
	RoomShader(int x, int y);
	virtual ~RoomShader();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
		*pd3dGraphicsRootSignature);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void AnimateObjects(float fTime);
};