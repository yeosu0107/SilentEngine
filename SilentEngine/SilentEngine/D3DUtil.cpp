#include "stdafx.h"
#include "D3DUtil.h"
#include <comdef.h>

DxException::DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & filename, int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	Filename(filename),
	LineNumber(lineNumber)
{
}

std::wstring DxException::ToString()const
{
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L"failed in" + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error : " + msg;
}

bool D3DUtil::IsKeyDown(int vkeyCode)
{
	return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
}

// 2진 파일 로드
ComPtr<ID3DBlob> D3DUtil::LoadBinary(const std::wstring& filename)
{
	ifstream fin(filename, ios::binary);

	fin.seekg(0, ios_base::end);
	ifstream::pos_type size = (int)fin.tellg();
	fin.seekg(0, ios_base::beg);

	ComPtr<ID3DBlob> d3dblob;
	ThrowIfFailed(D3DCreateBlob(size, d3dblob.GetAddressOf()));

	fin.read((char*)d3dblob->GetBufferPointer(), size);
	fin.close();

	return d3dblob;
}

// 디폴트 버퍼 생성 ( 변하지 않는 값 저장 버퍼 )
ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> d3dResourcedefaultBuffer;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(d3dResourcedefaultBuffer.GetAddressOf()))
	);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf()))
	);

	D3D12_SUBRESOURCE_DATA subResourceData = {};

	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dResourcedefaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST)
	);
	UpdateSubresources<1>(cmdList, d3dResourcedefaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(d3dResourcedefaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ)
	);

	return d3dResourcedefaultBuffer;
}

ComPtr<ID3DBlob> D3DUtil::CompileShader(
	const wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const string& entrypoint,
	const string& target)
{
	UINT compileFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> d3dBlobbyteCode = nullptr;
	ComPtr<ID3DBlob> d3dBloberrors;

	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &d3dBlobbyteCode, &d3dBloberrors);

	if (d3dBloberrors != nullptr)
		OutputDebugStringA((char*)d3dBloberrors->GetBufferPointer());

	ThrowIfFailed(hr);

	return d3dBlobbyteCode;
}