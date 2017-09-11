#pragma once
#include "GameObject.h"
#include "Camera.h"

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World;
};

//�ν��Ͻ� ����(���� ��ü�� ���� ��ȯ ��İ� ��ü�� ����)�� ���� ����ü
struct VS_VB_INSTANCE
{
	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4 m_xmcColor;
	bool	isDraw;
};

class CShader
{
protected:
	//���̴��� �����ϴ� ���� ��ü���� ����Ʈ(�迭)�̴�. 
	CGameObject **m_ppObjects = NULL;
	int m_nObjects = 0;
	int m_nReferences = 0;
	//���������� ���� ��ü���� ����Ʈ(�迭)�̴�. 
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

	//�������̴� ����Ʈ�ڵ� ����(������)
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	//�ʼ����̴� ����Ʈ�ڵ� ����(������
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	//���̴� �ҽ��ڵ� ������ & ����Ʈ �ڵ� ����ü�� ��ȯ
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
	//�ν��Ͻ� ���� ���ۿ� ���� ���� ���̴�. 
	ID3D12Resource *m_pd3dcbGameObjects = NULL;
	VS_VB_INSTANCE *m_pcbMappedGameObjects = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
};