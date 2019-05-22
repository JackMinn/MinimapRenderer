#pragma once

#include "D3D11/D3D11RHI.h"
#include "D3D11/D3D11ConstantBuffer.h"
#include "Camera.h"

// As of current I didnt create graphics API independant wrappers around the D3D11 Interfaces.
// This is something I would do in the future if I wanted to support multiple APIs.
// The D3D11 classes are designed with this flexibility in mind for future use cases. 

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

class RenderEngine {
public:
	// This must remain a simple constructor
	RenderEngine();
	~RenderEngine();
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(RenderEngine);

	bool Initialize(UINT32 screenWidth, UINT32 screenHeight, HWND hwnd);
	void Shutdown();

	bool RenderFrame();

	bool CreateTexture2DFromFile(const std::string& directory, const std::string& prefix, const std::string& file, Texture2D* texture2D) const
	{
		// Textures in the directory have a prefix and file format is not dds btut stx
		std::string actualFileName = directory + prefix + file.substr(0, file.size() - 3) + "stx";
		return m_RHI->CreateTexture2DFromFile(actualFileName, texture2D);
	}

	bool CreateMeshFromMemory(const std::vector<UINT32>& indexBufferData, const std::vector<VertexType>& vertexBufferData, IndexBuffer* indexBuffer, VertexBuffer* vertexBuffer) const {
		return m_RHI->CreateMeshFromMemory(indexBufferData, vertexBufferData, indexBuffer, vertexBuffer);
	}

private:
	bool CreateConstantBuffers();
	void BindConstantBuffers();
	
private:
	ComPtr<Camera> m_MainCamera;

	ComPtr<D3D11RHI> m_RHI;
	
	ComPtr<D3D11ConstantBuffer> m_drawCBuffer;
	ComPtr<D3D11ConstantBuffer> m_frameCBuffer;
	ComPtr<D3D11ConstantBuffer> m_applicationCBuffer;

	// This should not be here, later move to materials
	ComPtr<D3D11ConstantBuffer> m_globalsCBuffer;

	ComPtr<Shader> m_StandardDiffuseShader;
};
