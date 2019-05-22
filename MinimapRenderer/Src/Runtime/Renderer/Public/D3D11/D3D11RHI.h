#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "D3D11Util.h"

#include "SimpleMath.h"
#include "Misc/RefCounting.h"

#include "Engine/VertexBuffer.h"
#include "Engine/IndexBuffer.h"

class Texture2D;
class D3D11RHI : public RefCountedObject
{
public:
	D3D11RHI();
	~D3D11RHI() {}
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(D3D11RHI);

	bool Initialize(const uint32_t& screenWidth, const uint32_t& screenHeight, const bool& vsync, HWND const& hwnd, const bool& fullscreen, D3D11_VIEWPORT* const& viewport);
	void Shutdown();

	void BeginFrame(const DirectX::SimpleMath::Color& clearColor);
	void EndFrame();

	ID3D11Device* GetDevice() const { return m_Device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext.Get(); }

	void SetDepthTest(bool depthEnable, D3D11_COMPARISON_FUNC comparisonFunction);
	void SetStencilTest();
	void SetCullModeAndZBias();

	bool Render();

	void UpdateConstantBuffer(ID3D11Buffer* cBuffer, void* cpuBuffer);
	bool CreateTexture2DFromFile(const std::string& file, Texture2D* texture2D);
	bool CreateMeshFromMemory(const std::vector<UINT32>& indexBufferData, const std::vector<VertexType>& vertexBufferData, IndexBuffer* indexBuffer, VertexBuffer* vertexBuffer);


private:
	bool m_VsyncEnabled;
	uint32_t m_VideoCardMemory;
	char m_VideoCardDescription[128];

	ComPtr<IDXGIAdapter> m_Adapter = nullptr;
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_DeviceContext;
	ComPtr<IDXGISwapChain> m_SwapChain;

	ComPtr<ID3D11RenderTargetView> m_BackBufferRenderTargetView;
	ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	ComPtr<ID3D11RasterizerState> m_RasterState;

	bool InitializeMonitorProperties(const bool& vsync, HWND const& hwnd, uint32_t& numeratorOut, uint32_t& denominatorOut);
	bool InitializeDeviceAndSwapChain(const uint32_t& screenWidth, const uint32_t& screenHeight, const uint32_t& numerator, const uint32_t& denominator, const HWND& hwnd);
	bool InitializeBackBufferAndDepthBuffer(const uint32_t& screenWidth, const uint32_t& screenHeight);
	bool InitializeRasterizer();
};
