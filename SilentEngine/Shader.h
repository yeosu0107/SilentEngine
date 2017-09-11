#pragma once
#include "GameObject.h"
#include "Camera.h"

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World;
};

//인스턴스 정보(게임 객체의 월드 변환 행렬과 객체의 색상)를 위한 구조체
struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4 m_xmcColor;
	bool	isDraw;
};

class CShader
{
protected:
	//셰이더가 포함하는 게임 객체들의 리스트(배열)이다. 
	CGameObject **m_ppObjects = NULL;
	int m_nObjects = 0;
	int m_nReferences = 0;
	//파이프라인 상태 객체들의 리스트(배열)이다. 
	ID3D12PipelineState **m_ppd3dPipelineStates = NULL;
	int m_nPipelineStates = 0;

public:
	CShader();
	virtual ~CShader();

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	//정점셰이더 바이트코드 생성(컴파일)
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	//필셀셰이더 바이트코드 생성(컴파일
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	//셰이더 소스코드 컴파일 & 바이트 코드 구조체를 반환
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName,
		LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);


	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void ReleaseUploadBuffers();
	virtual void BuildObjects(ID3D12Device *pd3dDevice,
		ID3D12GraphicsCommandList *pd3dCommandList, void *pContext, CGameObject** pObject);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void updatePlayerPos(XMFLOAT3&) {}
	virtual void updatePlayerRot(XMFLOAT3&, float&) {}
	CGameObject** GetObjects() { return m_ppObjects; }
	int	getObjectsNum() const { return m_nObjects; }

	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition,
		XMFLOAT4X4& xmf4x4View, float *pfNearHitDistance) {
		return nullptr;
	}
};

class ObjectShader : public CShader
{
protected:

public:
	ObjectShader();
	virtual ~ObjectShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
		*pd3dGraphicsRootSignature);
	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition,
		XMFLOAT4X4& xmf4x4View, float *pfNearHitDistance);
};

class CInstancingShader : public ObjectShader
{
private:
	XMFLOAT3 playerPos;
	int n_draw = 0;
public:
	CInstancingShader();
	virtual ~CInstancingShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature
		*pd3dGraphicsRootSignature);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void  AnimateObjects(float fTimeElapsed);
	void updatePlayerPos(XMFLOAT3& player) { playerPos = player; }
protected:
	//인스턴스 정점 버퍼와 정점 버퍼 뷰이다. 
	ID3D12Resource *m_pd3dcbGameObjects = NULL;
	VS_VB_INSTANCE *m_pcbMappedGameObjects = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
};