#include "Engine/Mesh.h"

#include "Engine/RenderEngine.h"
extern RenderEngine g_RenderEngine;

bool Mesh::Initialize(const std::vector<VertexType>& vertexData, const std::vector<UINT32>& indexData) {
	m_VertexBuffer = new VertexBuffer();
	m_IndexBuffer = new IndexBuffer(indexData.size());

	if (g_RenderEngine.CreateMeshFromMemory(indexData, vertexData, m_IndexBuffer.Get(), m_VertexBuffer.Get())) {
		return true;
	}
	else {
		return false;
	}
}

bool Mesh::InitializeAsTriangle() {
	using namespace DirectX::SimpleMath;
	std::vector<VertexType> vertexData(3);
	std::vector<UINT32> indexData(3);

	vertexData[0].position = Vector4(-1.0f, -1.0f, 0.0f, 1.0f);  // Bottom left.
	vertexData[0].normal = Vector4::One;
	vertexData[0].uv = Vector4::One;

	vertexData[1].position = Vector4(0.0f, 1.0f, 0.0f, 1.0f);  // Top middle.
	vertexData[1].normal = Vector4::One;
	vertexData[1].uv = Vector4::One;

	vertexData[2].position = Vector4(1.0f, -1.0f, 0.0f, 1.0f);  // Bottom right.
	vertexData[2].normal = Vector4::One;
	vertexData[2].uv = Vector4::One;

	indexData[0] = 0;  // Bottom left.
	indexData[1] = 1;  // Top middle.
	indexData[2] = 2;  // Bottom right.

	m_VertexBuffer = new VertexBuffer();
	m_IndexBuffer = new IndexBuffer(indexData.size());

	if (g_RenderEngine.CreateMeshFromMemory(indexData, vertexData, m_IndexBuffer.Get(), m_VertexBuffer.Get())) {
		return true;
	}
	else {
		return false;
	}
}
