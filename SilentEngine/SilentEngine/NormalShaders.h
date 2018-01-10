#pragma once
#include "Shader.h"

class CIlluminatedTexturedNormalShader : public CIlluminatedTexturedShader
{
private:

public:
	CIlluminatedTexturedNormalShader();
	~CIlluminatedTexturedNormalShader();

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

};