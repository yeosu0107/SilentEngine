#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{

}


CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	//그래픽 루트 시그너쳐를 생성한다. 
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);



	//셰이더 생성
	m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	CInstancingShader* tshader = new CInstancingShader();
	tshader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	tshader->BuildObjects(pd3dDevice, pd3dCommandList);

	m_ppObjects = tshader->GetObjects();
	m_nObjects = tshader->getObjectsNum();
	m_ppShaders[0] = tshader;
	
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();

	if (m_ppShaders) {
		for (int i = 0; i < m_nShaders; i++) {
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
		}
		delete[] m_ppShaders;
	}
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{

	return false;
}


void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; ++i) {
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}
}



void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (pCamera)
		pCamera->UpdateShaderVariables(pd3dCommandList);

	for(int i=0; i<m_nShaders; ++i)
		m_ppShaders[i]->Render(pd3dCommandList, pCamera);
}


bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CScene::ReleaseUploadBuffers() {
	if (m_ppShaders) {
		for (int j = 0; j < m_nShaders; j++)
			m_ppShaders[j]->ReleaseUploadBuffers();
	}
}

ID3D12RootSignature *CScene::GetGraphicsRootSignature() {
	return m_pd3dGraphicsRootSignature;
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice) {
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	//루트 파라미터 설정
	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 16;
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //b0: Player
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //b1: Camera
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 0; //t0
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;

	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;

	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;

	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void
			**)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();

	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
	return(pd3dGraphicsRootSignature);
}


void CScene::CrashObject(BoundingOrientedBox& player)
{
	for (int i = 0; i < m_nObjects; ++i) {
		if (!m_ppObjects[i]->GetLive())
			continue;

		if (m_ppObjects[i]->getOOBB()->Intersects(player)) {
			printf("Crash! %d번째 오브젝트\n", i);
		}
	}
}