#include "stdafx.h"
#include "NormalShaders.h"

CIlluminatedTexturedNormalShader::CIlluminatedTexturedNormalShader()
{
}

CIlluminatedTexturedNormalShader::~CIlluminatedTexturedNormalShader()
{
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedTexturedNormalShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

void CIlluminatedTexturedNormalShader::CreateShader(ID3D12Device * pd3dDevice, ID3D12RootSignature * pd3dGraphicsRootSignature, UINT nRenderTargets)
{
}

D3D12_SHADER_BYTECODE CIlluminatedTexturedNormalShader::CreateVertexShader(ID3DBlob ** ppd3dShaderBlob)
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE CIlluminatedTexturedNormalShader::CreatePixelShader(ID3DBlob ** ppd3dShaderBlob)
{
	return D3D12_SHADER_BYTECODE();
}
