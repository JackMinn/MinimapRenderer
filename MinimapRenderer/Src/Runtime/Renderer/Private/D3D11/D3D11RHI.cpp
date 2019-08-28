//https://stackoverflow.com/questions/13479259/read-pixel-data-from-render-target-in-d3d11
//https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
//https://docs.microsoft.com/en-us/windows/desktop/api/dxgi/nf-dxgi-idxgiswapchain-getbuffer

#include "D3D11/D3D11RHI.h"
#include "DDSTextureLoader.h"
#include "Engine/Texture2D.h"

D3D11RHI::D3D11RHI() : 
	m_Adapter(nullptr), 
	m_Device(nullptr),
	m_DeviceContext(nullptr),
	m_SwapChain(nullptr),
	m_BackBufferRenderTargetView(nullptr),
	m_DepthStencilBuffer(nullptr),
	m_DepthStencilView(nullptr),
	m_DepthStencilState(nullptr),
	m_RasterState(nullptr)
{
}


bool D3D11RHI::Initialize(const UINT32& screenWidth, const UINT32& screenHeight, const bool& vsync, HWND const& hwnd, const bool& fullscreen, D3D11_VIEWPORT* const& viewport)
{
	UINT32 numerator, denominator;

	DebugLog("s", "Initializing Monitor Properties");
	if (!InitializeMonitorProperties(vsync, hwnd, numerator, denominator))
	{
		return false;
	}

	DebugLog("s", "Initializing Device and Swapchain");
	if (!InitializeDeviceAndSwapChain(screenWidth, screenHeight, numerator, denominator, hwnd))
	{
		return false;
	}

	DebugLog("s", "Initializing Back Buffer and Depth Buffer");
	if (!InitializeBackBufferAndDepthBuffer(screenWidth, screenHeight))
	{
		return false;
	}

	DebugLog("s", "Initializing Rasterizer");
	if (!InitializeRasterizer())
	{
		return false;
	}

	// Set all device context properties.
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
	m_DeviceContext->OMSetRenderTargets(1, m_BackBufferRenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	m_DeviceContext->RSSetState(m_RasterState.Get());
	m_DeviceContext->RSSetViewports(1, viewport);

	return true;
}

void D3D11RHI::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}
}

void D3D11RHI::BeginFrame(const DirectX::SimpleMath::Color& clearColor)
{
	// Clear the back buffer and depth buffer.
	m_DeviceContext->ClearRenderTargetView(m_BackBufferRenderTargetView.Get(), reinterpret_cast<const float*>(&clearColor));
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3D11RHI::EndFrame()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_VsyncEnabled)
	{
		m_SwapChain->Present(1, 0);
	}
	else
	{
		m_SwapChain->Present(0, 0);
	}
}

bool D3D11RHI::InitializeMonitorProperties(const bool& vsync, HWND const& hwnd, UINT32& numeratorOut, UINT32& denominatorOut)
{
	HRESULT result;
	ComPtr<IDXGIFactory1> factory;
	ComPtr<IDXGIOutput> adapterOutput;
	unsigned int numModes;
	unsigned int numerator = 0, denominator = 0;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;

	m_VsyncEnabled = vsync;

	result = CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(result))
	{
		return false;
	}

	// Start with the first adapter and then see if any better ones are available
	result = factory->EnumAdapters(0, m_Adapter.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	
	ComPtr<IDXGIAdapter> tempAdapter;
	DXGI_ADAPTER_DESC tempAdapterDesc;
	for (UINT32 adapterIndex = 1; factory->EnumAdapters(adapterIndex, tempAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
	{
		m_Adapter->GetDesc(&adapterDesc);
		tempAdapter->GetDesc(&tempAdapterDesc);
		if (tempAdapterDesc.DedicatedVideoMemory > adapterDesc.DedicatedVideoMemory) {
			m_Adapter = tempAdapter;
		}
	}

	// Enumerate the primary adapter output (monitor).
	result = m_Adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create and fill a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	DEBUG_ASSERT(displayModeList);

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	HMONITOR currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO activeMonitorInfo;
	activeMonitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(currentMonitor, (LPMONITORINFO)&activeMonitorInfo);
	long activeMonitorWidth = activeMonitorInfo.rcMonitor.right - activeMonitorInfo.rcMonitor.left;
	long activeMonitorHeight = activeMonitorInfo.rcMonitor.bottom - activeMonitorInfo.rcMonitor.top;
	DebugLog("sisi", "Active monitor height is: ", activeMonitorHeight, ", Active monitor width is: ", activeMonitorWidth);

	// Now go through all the display modes and find the one that matches the active monitor width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (UINT32 i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)activeMonitorWidth && displayModeList[i].Height == (unsigned int)activeMonitorHeight)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	if (numerator == 0 && denominator == 0)
	{
		DebugLog("s", "Did not find a display mode matching the window resolutions");
		return false;
	}

	result = m_Adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	sprintf_s(m_VideoCardDescription, "%ws", adapterDesc.Description);
	DebugLog("s", m_VideoCardDescription);

	delete[] displayModeList;
	displayModeList = nullptr;

	numeratorOut = numerator;
	denominatorOut = denominator;

	return true;
}


bool D3D11RHI::InitializeDeviceAndSwapChain(const UINT32& screenWidth, const UINT32& screenHeight, const UINT32& numerator, const UINT32& denominator, const HWND& hwnd)
{
	HRESULT result;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// For now we use just a single backbuffer, can consider multi-buffering later
	swapChainDesc.BufferCount = 1;

	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	if (m_VsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// This is crashing us, probably due to adapter version or swapchain version
	// Might need to revist this for when we readback from the swap chain backbuffer for printscreening.
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;
	assert(m_Adapter != nullptr);

	DXGI_ADAPTER_DESC adapterDesc;
	result = m_Adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	DebugLog("si", "Video card memory is: ", (uint64_t)(adapterDesc.DedicatedVideoMemory / 1024 / 1024));

	//either use the default adapter, or use the enumerated adapter (where later we can search for the most powerful GPU)
#if 0
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, NULL, &m_DeviceContext);
#else
	result = D3D11CreateDeviceAndSwapChain(m_Adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, m_SwapChain.GetAddressOf(), m_Device.GetAddressOf(), NULL, m_DeviceContext.GetAddressOf());
#endif

	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3D11RHI::InitializeBackBufferAndDepthBuffer(const UINT32& screenWidth, const UINT32& screenHeight)
{
	HRESULT result;
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
		if (FAILED(result))
		{
			return false;
		}

		// Create the render target view with the back buffer pointer.
		result = m_Device->CreateRenderTargetView(backBuffer.Get(), NULL, m_BackBufferRenderTargetView.GetAddressOf());
		if (FAILED(result))
		{
			return false;
		}
	}

	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	//Initialize and fill the depth/stencil buffer description, then create the buffer from the description.
	ZeroMemory(&depthStencilBufferDesc, sizeof(depthStencilBufferDesc));
	depthStencilBufferDesc.Width = screenWidth;
	depthStencilBufferDesc.Height = screenHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;
	result = m_Device->CreateTexture2D(&depthStencilBufferDesc, NULL, m_DepthStencilBuffer.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Initialize and fill the depth/stencil view description, then create the view from the description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	result = m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, m_DepthStencilView.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Initialize and fill the description of the depth/stencil state.
	ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));

	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilStateDesc.StencilEnable = true;
	depthStencilStateDesc.StencilReadMask = 0xFF;
	depthStencilStateDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_Device->CreateDepthStencilState(&depthStencilStateDesc, m_DepthStencilState.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3D11RHI::InitializeRasterizer()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterDesc;

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	//rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_Device->CreateRasterizerState(&rasterDesc, m_RasterState.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool D3D11RHI::Render()
{
	return true;
}

void D3D11RHI::UpdateConstantBuffer(ID3D11Buffer* cBuffer, void* cpuBuffer)
{
	m_DeviceContext->UpdateSubresource(cBuffer, 0, nullptr, cpuBuffer, 0, 0);
}

bool D3D11RHI::CreateTexture2DFromFile(const std::string& file, Texture2D* texture2D)
{
	std::wstring fileW(file.begin(), file.end());
	HRESULT hres = DirectX::CreateDDSTextureFromFile(m_Device.Get(), fileW.c_str(), texture2D->m_Resource.GetAddressOf(), texture2D->m_SRV.GetAddressOf(), true);

	if (FAILED(hres))
	{
		return false;
	} 
	else 
	{
		ID3D11Texture2D* temp;
		HRESULT hres = texture2D->m_Resource->QueryInterface(IID_PPV_ARGS(&temp));
		if (FAILED(hres)) {
			if (temp) temp->Release();
			return false;
		}
		else {
			if (temp) temp->Release();

			return true;
		}
	}
}

bool D3D11RHI::CreateMeshFromMemory(const std::vector<UINT32>& indexBufferData, const std::vector<VertexType>& vertexBufferData, IndexBuffer* indexBuffer, VertexBuffer* vertexBuffer) {
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexBufferData.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertexBufferData.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = m_Device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer->GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT32) * indexBufferData.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indexBufferData.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = m_Device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer->GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}
