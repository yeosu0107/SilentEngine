#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
#include "Timer.h"
#include "Camera.h"
#include "UIShaders.h"

#include "..\Model\ModelObject.h"
#include "..\Room\Room.h"

#include "Shaders.h"
#include "LightObjects.h"
#include "..\Shaders\PlayerShader.h"
#include "..\Shaders\ProjectileShader.h"
#include "..\Model\InstanceModelShader.h"

const UINT FADE_OFF		= 0;
const UINT FADE_IN		= 1;
const UINT FADE_OUT		= 2;
const UINT FADE_END		= 3;

// Scene 
class Scene
{
public:
	Scene();
	~Scene();

public:
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {};
	virtual void Update(const Timer& gt) {};
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) ;
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList) ;

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
	vector<UIShaders*>											m_ppUIShaders;

protected:
	unique_ptr<Camera>											m_Camera = nullptr;

	BasePhysX*		m_physics;
	UINT				m_nUIShaders;
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
	virtual void RenderUI(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);
	virtual void CreateShadowMap(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList, int index = 0);
	virtual void CalculateLightMatrix(VS_CB_CAMERA_INFO& cameraInfo, int index, float offset);

	void BuildLightsAndMaterials();
	void BuildFog();

	virtual bool OnKeyboardInput(const Timer& gt, HWND& hWin);
	virtual bool OnMouseDown(HWND& hWin, WPARAM btnState, UINT nMessageID, int x, int y);
	virtual bool OnMouseUp(HWND& hWin, WPARAM btnState, UINT nMessageID, int x, int y);
	virtual bool OnMouseMove(float x, float y);
	
	GameObject* GetPlayer() { return m_testPlayer; }

	void RoomChange();
	void RoomSetting();
	void RoomFade();
	
	void CaptureCursor();

protected:
	PlayerShader*											m_playerShader = nullptr;
	InstanceModelShader*								m_gateShader = nullptr;
	BillboardShader*										m_EffectShaders = nullptr;
	BillboardShader*										m_hitEffectShaders = nullptr;

	ProjectileShader*									m_Projectile = nullptr;
	FadeEffectShader*									m_pFadeEffectShader = nullptr;

	Room**													m_Room = nullptr;
	
	UINT														m_nProjectile = 0;
	UINT														m_nRoom = 0;
	int**														m_virtualMap = nullptr;

	UINT														m_nowRoom;

	Door														m_isRoomChange;
	
	LightManagement*												m_pLights;
	//unique_ptr<UploadBuffer<LIGHTS>>			m_pd3dcbLights = nullptr;

	MATERIALS*											m_pMaterials;
	unique_ptr<UploadBuffer<MATERIALS>>	m_pd3dcbMaterials = nullptr;

	CB_FOG_INFO*										m_pFog;
	unique_ptr<UploadBuffer<CB_FOG_INFO>>				m_pd3dcbFog = nullptr;

	GameObject*											m_testPlayer=nullptr;

	UINT														m_testTimer = 0;

	UINT														m_changeFade = 0;

	//조작관련
	bool														m_bMouseCapture = true;
	POINT													m_ptOldCursorPos;
	float														m_fMouseSensitive = 4.5f;	// 마우스 민감도
};

class GameScene : public Scene
{

};

class MainScene : public Scene
{
public:
	MainScene();
	~MainScene();
public:
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {};
	virtual void Update(const Timer& gt) {};
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {};
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList) {};
};