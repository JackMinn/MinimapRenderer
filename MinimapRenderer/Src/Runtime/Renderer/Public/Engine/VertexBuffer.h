#pragma once

#include "Misc/RefCounting.h"

#include <d3d11.h>
#include "SimpleMath.h"

__declspec(align(16)) struct VertexType
{
	DirectX::SimpleMath::Vector4 position;
	DirectX::SimpleMath::Vector4 normal;
	DirectX::SimpleMath::Vector4 uv;
};


class VertexBuffer : public RefCountedObject {
public:
	VertexBuffer() :
		m_Buffer(nullptr)
	{}

	~VertexBuffer()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(VertexBuffer);

	ID3D11Buffer* Get() const { return m_Buffer.Get(); }
	ID3D11Buffer** GetAddressOf() { return m_Buffer.GetAddressOf(); }

private:
	ComPtr<ID3D11Buffer> m_Buffer;
};