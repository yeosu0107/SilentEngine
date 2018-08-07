#include "stdafx.h"
#include "D3DUtil.h"
#include <comdef.h>

bool HDR_ON = true;
bool BLOOM_ON = true;
BOOL fullScreenState = false;

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

// 빈텍스쳐, 메모리를 할당 하는 부분
ComPtr<ID3D12Resource> D3DUtil::CreateTexture2DResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue)
{
	ComPtr<ID3D12Resource> pd3dTexture = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dTextureResourceDesc;
	::ZeroMemory(&d3dTextureResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dTextureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dTextureResourceDesc.Alignment = 0;
	d3dTextureResourceDesc.Width = nWidth;
	d3dTextureResourceDesc.Height = nHeight;
	d3dTextureResourceDesc.DepthOrArraySize = 1;
	d3dTextureResourceDesc.MipLevels = 1;
	d3dTextureResourceDesc.Format = dxgiFormat;
	d3dTextureResourceDesc.SampleDesc.Count = 1;
	d3dTextureResourceDesc.SampleDesc.Quality = 0;
	d3dTextureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dTextureResourceDesc.Flags = d3dResourceFlags;

	// 텍스쳐를 나타내는 메모리만 할당 
	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dTextureResourceDesc, d3dResourceStates, pd3dClearValue, __uuidof(ID3D12Resource), (void **)&pd3dTexture);

	return(pd3dTexture);
}

void D3DUtil::CreateUnorderedAccessResource(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, XMUINT2& size, ComPtr<ID3D12Resource>& buffer, ComPtr<ID3D12Resource>& readbackBuffer, UINT type)
{
	switch (type) {
	case UAFloatBuffer:
		ThrowIfFailed(pd3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(
				size.x,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(buffer.GetAddressOf()))
		);

		ThrowIfFailed(pd3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size.x),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(readbackBuffer.GetAddressOf()))
		);
		break;

	case UATexBuffer:
		buffer = D3DUtil::CreateTexture2DResource(pd3dDevice, pd3dCommandList, size.x, size.y, 
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr);
		readbackBuffer = D3DUtil::CreateTexture2DResource(pd3dDevice, pd3dCommandList, size.x, size.y,
			DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);
		break;
	}
}


ComPtr<ID3D12Resource> D3DUtil::CreateTextureResourceFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, const wchar_t *pszFileName, ID3D12Resource* ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ComPtr<ID3D12Resource> pd3dTexture = NULL;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &pd3dTexture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dTextureResourceDesc = pd3dTexture->GetDesc();
	UINT nSubResources = (UINT)vSubresources.size();
	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture.Get(), 0, nSubResources);
	//	UINT nSubResources = d3dTextureResourceDesc.DepthOrArraySize * d3dTextureResourceDesc.MipLevels;
	//	UINT64 nBytes = 0;
	//	pd3dDevice->GetCopyableFootprints(&d3dTextureResourceDesc, 0, nSubResources, 0, NULL, NULL, NULL, &nBytes);

	D3D12_RESOURCE_DESC d3dBufferResourceDesc;
	::ZeroMemory(&d3dBufferResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap에는 텍스쳐를 생성할 수 없음
	d3dBufferResourceDesc.Alignment = 0;
	d3dBufferResourceDesc.Width = nBytes;
	d3dBufferResourceDesc.Height = 1;
	d3dBufferResourceDesc.DepthOrArraySize = 1;
	d3dBufferResourceDesc.MipLevels = 1;
	d3dBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dBufferResourceDesc.SampleDesc.Count = 1;
	d3dBufferResourceDesc.SampleDesc.Quality = 0;
	d3dBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void **)&ppd3dUploadBuffer);

	//UINT nSubResources = (UINT)vSubresources.size();
	//D3D12_SUBRESOURCE_DATA *pd3dSubResourceData = new D3D12_SUBRESOURCE_DATA[nSubResources];
	//for (UINT i = 0; i < nSubResources; i++) pd3dSubResourceData[i] = vSubresources.at(i);

	//	std::vector<D3D12_SUBRESOURCE_DATA>::pointer ptr = &vSubresources[0];
	UINT64 nBytesUpdated = ::UpdateSubresources(pd3dCommandList, pd3dTexture.Get(), ppd3dUploadBuffer, 0, 0, nSubResources, &vSubresources[0]);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture.Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//	delete[] pd3dSubResourceData;

	return(pd3dTexture);
}

void D3DUtil::CalculateTangentArray(long vertexCount, const XMFLOAT3 *vertex, XMFLOAT3 *normal,
	const XMFLOAT2 *texcoord, long triangleCount, const UINT *indeies, XMFLOAT3 *tangent)
{
	XMFLOAT3 *tan1 = new XMFLOAT3[vertexCount * 2];
	XMFLOAT3 *tan2 = tan1 + vertexCount;
	::ZeroMemory(tan1, vertexCount * sizeof(XMFLOAT3) * 2);

	for (long a = 0; a < triangleCount; a++)
	{
		UINT i1 = indeies[0];
		UINT i2 = indeies[1];
		UINT i3 = indeies[2];

		const XMFLOAT3& v1 = vertex[i1];
		const XMFLOAT3& v2 = vertex[i2];
		const XMFLOAT3& v3 = vertex[i3];

		const XMFLOAT2& w1 = texcoord[i1];
		const XMFLOAT2& w2 = texcoord[i2];
		const XMFLOAT2& w3 = texcoord[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] = Vector3::Add(tan1[i1], sdir);
		tan1[i2] = Vector3::Add(tan1[i2], sdir);
		tan1[i3] = Vector3::Add(tan1[i3], sdir);

		tan2[i1] = Vector3::Add(tan2[i1], tdir);
		tan2[i2] = Vector3::Add(tan2[i2], tdir);
		tan2[i3] = Vector3::Add(tan2[i3], tdir);

		indeies += 3;
	}

	for (long a = 0; a < vertexCount; a++)
	{
		XMFLOAT3& n = normal[a];
		XMFLOAT3& t = tan1[a];

		// Gram-Schmidt orthogonalize
		tangent[a] = Vector3::Normalize(Vector3::Subtract(t, Vector3::ScalarProduct(n, Vector3::DotProduct(n, t), false), false));

		// Calculate handedness
		//tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
	}

	delete[] tan1;
}
