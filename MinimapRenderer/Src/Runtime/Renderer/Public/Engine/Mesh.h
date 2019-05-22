#pragma once

#include "Misc/RefCounting.h"

#include <d3d11.h>
#include "SimpleMath.h"

#include "Engine/VertexBuffer.h"
#include "Engine/IndexBuffer.h"

class Mesh : public RefCountedObject {
public:
	Mesh() :
		m_VertexBuffer(nullptr),
		m_IndexBuffer(nullptr)
	{}

	~Mesh()
	{}

	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(Mesh);

	bool Initialize(const std::vector<VertexType>& vertexData, const std::vector<UINT32>& indexData);
	bool InitializeAsTriangle();

	VertexBuffer* GetVertexBuffer() const { return m_VertexBuffer.Get(); }
	IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer.Get(); }

private:
	ComPtr<VertexBuffer> m_VertexBuffer;
	ComPtr<IndexBuffer> m_IndexBuffer;
};