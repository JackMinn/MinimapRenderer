#pragma once

#include "Misc/RefCounting.h"

class IndexBuffer : public RefCountedObject {
public:
	IndexBuffer(UINT32 indexCount) :
		m_Buffer(nullptr),
		m_IndexCount(indexCount)
	{}

	~IndexBuffer()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(IndexBuffer);

	ID3D11Buffer* Get() const { return m_Buffer.Get(); }
	ID3D11Buffer** GetAddressOf() { return m_Buffer.GetAddressOf(); }

public: 
	UINT32 m_IndexCount;

private:
	ComPtr<ID3D11Buffer> m_Buffer;
};