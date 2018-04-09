#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
#include "Timer.h"
#include "Camera.h"

#include "..\Model\ModelObject.h"
#include "..\Room\Room.h"

#include "Shaders.h"
#include "..\Shaders\PlayerShader.h"
#include "..\Shaders\ProjectileShader.h"
#include "..\Model\InstanceModelShader.h"




// Scene 
class Scene
{
public:
	Scene();
	~Scene();

public:
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void Update(const Timer& gt) = 0;;
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);

	virtual Camera* GetCamera() { return m_Camera.get(); }

protected:
	int									m_CurrFrameResourceIndex = 0;
	UINT								m_nCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature>			m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>		m_SrvDescriptorHeap = nullptr;

	shared_ptr<unordered_map<string, unique_ptr<MeshGeometry>>>			m_Geometries;
	unordered_map<string, ComPtr<ID3DBlob>>						m_Shaders;
	unordered_map<string, ComPtr<ID3D12PipelineState>>			m_PSOs;
	vector<D3D12_INPUT_ELEMENT_DESC>							m_InputLayout;

	unique_ptr<UploadBuffer<CB_GAMEOBJECT_INFO>>					m_ObjectCB;

protected:
	unique_ptr<Camera>											m_Camera = nullptr;

	BasePhysX*		m_physics;
};

class TestScene : public Scene
{
public:
	TestScene();
	~TestScene();

public:
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void CreateShaderVariables(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void UpdateShaderVarialbes();
	virtual void Update(const Timer& gt);
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);
	virtual void RenderShadow(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);
	virtual void CreateShadowMap(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);
	virtual void CalculateLightMatrix(VS_CB_CAMERA_INFO& cameraInfo);

	void BuildLightsAndMaterials();

	virtual bool OnKeyboardInput(const Timer& gt, UCHAR *pKeysBuffer);
	virtual bool OnMouseDown(HWND& hWin, WPARAM btnState, int x, int y);
	virtual bool OnMouseUp(HWND& hWin, WPARAM btnState, int x, int y);
	virtual bool OnMouseMove(HWND& hWin, WPARAM btnState, float x, float y);
	
	virtual void RoomChange();
protected:
	PlayerShader*					m_playerShader = nullptr;
	InstanceModelShader*		m_gateShader = nullptr;
	BillboardShader*				m_EffectShaders = nullptr;
	ProjectileShader*			m_Projectile = nullptr;

	Room**							m_Room = nullptr;
	
	UINT								m_nShaders = 0;
	UINT								m_nProjectile = 0;
	UINT								m_nRoom = 0;
	UINT								m_nowRoom;

	Door								m_isRoomChange;
	
	LIGHTS*												m_pLights;
	unique_ptr<UploadBuffer<LIGHTS>>			m_pd3dcbLights = nullptr;

	MATERIALS*											m_pMaterials;
	unique_ptr<UploadBuffer<MATERIALS>>	m_pd3dcbMaterials = nullptr;

	GameObject*		m_testPlayer=nullptr;
	GameObject**	m_Enemys=nullptr;
	UINT					m_nEnemy = 0;

	UINT					m_testTimer = 0;

	float					m_fTestAngle = 1.0f;
};

class GameScene : public Scene
{

};

class MainScene : public Scene
{

};