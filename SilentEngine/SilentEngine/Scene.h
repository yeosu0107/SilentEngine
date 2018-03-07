#pragma once
#include "D3DUtil.h"
#include "Mesh.h"
#include "Shaders.h"
#include "FrameResouce.h"
#include "Camera.h"
#include "Timer.h"


// Scene 
class Scene
{
public:
	Scene();
	~Scene();

public:
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void BuildDescriptorHeaps(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void BuildShadersAndInputLayout(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void BuildSceneGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void BuildPSOs(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) = 0;
	virtual void Update(const Timer& gt) = 0;
	//virtual void BuildFrameResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	//virtual void BuildMaterials(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	//virtual void BuildRenderItems(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);

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
protected:

};

class TestScene : public Scene
{
public:
	TestScene() {};
	~TestScene() {};

public:
	virtual void BuildBoxGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildRootSignature(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildDescriptorHeaps(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildShadersAndInputLayout(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildSceneGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildPSOs(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildConstantBuffers(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Update(const Timer& gt);
	//virtual void BuildFrameResources(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	//virtual void BuildMaterials(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	//virtual void BuildRenderItems(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void BuildScene(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	virtual void Render(ID3D12Device * pDevice, ID3D12GraphicsCommandList * pCommandList);
		
protected:
	unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;
	unique_ptr<Shaders> m_pShaders = nullptr;
	unique_ptr<Camera> m_Camera = nullptr;
	
};

class GameScene : public Scene
{

};

class MainScene : public Scene
{

};