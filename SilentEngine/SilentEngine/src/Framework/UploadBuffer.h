#pragma once

#include "D3DUtil.h"



template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* pDevice, UINT nElementCount, bool isConstantBuffer) : 
		m_bIsConstantBuffer(isConstantBuffer)
    {
        m_nElementByteSize = sizeof(T);

        if(isConstantBuffer)
			m_nElementByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(T));

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_nElementByteSize * nElementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_pUploadBuffer)));

        ThrowIfFailed(m_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedData)));

    }

    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if(m_pUploadBuffer != nullptr)
			m_pUploadBuffer->Unmap(0, nullptr);

		m_pMappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return m_pUploadBuffer.Get();
    }

    void CopyData(int elementIndex, T& data)
    {
        memcpy(&m_pMappedData[elementIndex*m_nElementByteSize], &data, sizeof(T));
    }

private:
	ComPtr<ID3D12Resource> m_pUploadBuffer;
    BYTE* m_pMappedData = nullptr;

    UINT m_nElementByteSize = 0;
    bool m_bIsConstantBuffer = false;
};