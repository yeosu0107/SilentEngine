#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
#include "Shaders.h"
//#include "FrameResouce.h"
#include "Camera.h"
#include "Timer.h"
#include "..\PhysX\BasePhysX.h"


#define MAX_LIGHTS	  8
#define MAX_MATERIALS 8

#define POINT_LIGHT				1
#define SPOT_LIGHT				2
#define DIRECTIONAL_LIGHT		3


struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;	// 광원의 위치 
	float 					m_fFalloff;		// Direction 조명에서 원의 바깥쪽으로 일어나는 감쇠 효과
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;		// 조명 온오프
	int						m_nType;		// 조명 종류
	float					m_fRange;		// 조명 길이
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
};

struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;	// 앰비언트 반사 색상 
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

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
	//vector<unique_ptr<FrameResource>>	m_FrameResources;
	//FrameResource*					m_pCurrentFrameResource = nullptr;
	
	int									m_CurrFrameResourceIndex = 0;
	UINT								m_nCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature>			m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>		m_SrvDescriptorHeap = nullptr;

	shared_ptr<unordered_map<string, unique_ptr<MeshGeometry>>>			m_Geometries;
	//unordered_map<string, unique_ptr<Material>>				m_Materials;
	//unordered_map<string, unique_ptr<Texture>>				m_Textures;
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
	
	void BuildLightsAndMaterials();

	virtual bool OnKeyboardInput(const Timer& gt, UCHAR *pKeysBuffer);
	virtual bool OnMouseDown(HWND& hWin, WPARAM btnState, int x, int y);
	virtual bool OnMouseUp(HWND& hWin, WPARAM btnState, int x, int y);
	virtual bool OnMouseMove(HWND& hWin, WPARAM btnState, float x, float y);
		
protected:
	//unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;
	Shaders** m_ppShaders = nullptr;
	
	UINT m_nShaders = 0;
	
	LIGHTS*									m_pLights;
	unique_ptr<UploadBuffer<LIGHTS>>		m_pd3dcbLights = nullptr;

	MATERIALS*								m_pMaterials;
	unique_ptr<UploadBuffer<MATERIALS>>		m_pd3dcbMaterials = nullptr;

	GameObject* m_testPlayer;
};

class GameScene : public Scene
{

};

class MainScene : public Scene
{

};