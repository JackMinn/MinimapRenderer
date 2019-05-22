#include "Engine/RenderEngine.h"

#include <chrono>

#include "AssetManager.h"
#include "Fixture.h"
#include "Engine/VertexBuffer.h"
#include "Engine/IndexBuffer.h"
#include "Engine/Texture2D.h"
extern AssetManager g_AssetManager;

RenderEngine::RenderEngine() :
	m_RHI(nullptr),
	m_drawCBuffer(nullptr),
	m_frameCBuffer(nullptr),
	m_applicationCBuffer(nullptr),
	m_MainCamera(nullptr)
{}

RenderEngine::~RenderEngine()
{

}

bool RenderEngine::Initialize(UINT32 screenWidth, UINT32 screenHeight, HWND hwnd)
{
	D3D11RHI* RHI = new D3D11RHI();
	m_RHI = RHI;

	m_MainCamera = new Camera(screenWidth, screenHeight);

	bool result;
	result = m_RHI->Initialize(screenWidth, screenHeight, false, hwnd, false, &m_MainCamera->m_ViewPort);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Rendering Hardware Interface", L"Error", MB_OK);
		return false;
	}

	result = CreateConstantBuffers();
	if (!result) {
		MessageBox(hwnd, L"Could not create Constant Buffers", L"Error", MB_OK);
		return false;
	}
	BindConstantBuffers();

	m_StandardDiffuseShader = new Shader();
	result = m_StandardDiffuseShader->Initialize(m_RHI->GetDevice(), "shaders/StandardDiffuse.hlsl", "shaders/StandardDiffuse.hlsl");
	if (!result) {
		MessageBox(hwnd, L"Could not compile Standard Diffuse Shader", L"Error", MB_OK);
		return false;
	}

	return true;
}

void RenderEngine::Shutdown()
{
	m_RHI->Shutdown();
}

// Per Application Frame must be updated first elsewhere
// Per Frame is done here once on function call
// Per Draw is done for each loop over all instances
bool RenderEngine::RenderFrame() {
	// THIS SHOULD BE MOVED OUT LATER TO OCCUR ONLY WHEN NEEDED ===============================================================
	// It seems like the GPU projection matrix might be wrong
	DirectX::SimpleMath::Matrix projectionMatrix;
	DirectX::SimpleMath::Matrix invProjectionMatrix;
	m_MainCamera->BuildProjectionMatrix();
	m_MainCamera->GetGPUProjectionMatrix(projectionMatrix);
	m_MainCamera->GetGPUInvProjectionMatrix(invProjectionMatrix);

	void* applicationBuffer = malloc(CB_APPLICATION_SIZE);
	memcpy(applicationBuffer, &projectionMatrix, sizeof(projectionMatrix));
	memcpy((void*)((char*)applicationBuffer + sizeof(projectionMatrix)), &invProjectionMatrix, sizeof(invProjectionMatrix));

	m_RHI->UpdateConstantBuffer(m_applicationCBuffer.Get()->Get(), applicationBuffer);
	free(applicationBuffer);
	// ========================================================================================================================

	m_MainCamera->Update();
	m_RHI->BeginFrame(m_MainCamera->m_ClearColor);

	DirectX::SimpleMath::Matrix viewMatrix;
	void* frameBuffer = malloc(CB_FRAME_SIZE);
	m_MainCamera->BuildViewMatrix();
	m_MainCamera->GetGPUViewMatrix(viewMatrix);
	DirectX::SimpleMath::Vector4 cameraPos = { m_MainCamera->m_Transform.GetPosition().x, m_MainCamera->m_Transform.GetPosition().y, m_MainCamera->m_Transform.GetPosition().z, 1.0f };
	memcpy(frameBuffer, &viewMatrix, sizeof(viewMatrix));
	void* frameBufferIndex1 = (void*)((char*)frameBuffer + sizeof(viewMatrix));
	memcpy(frameBufferIndex1, &cameraPos, sizeof(cameraPos));

	m_RHI->UpdateConstantBuffer(m_frameCBuffer.Get()->Get(), frameBuffer);

	free(frameBuffer);

	// SHOULD NOT BE HERE, WILL LATER BE MOVED TO MATERIALS =======================
	DirectX::SimpleMath::Vector4 globals = DirectX::SimpleMath::Vector4::One;
	m_RHI->UpdateConstantBuffer(m_globalsCBuffer.Get()->Get(), (void*)(&globals));
	// ============================================================================

	// LOOP OVER ASSET MANAGER TO DO BINDINGS OF Model Matrix and Texture 

	// Shit shouldnt be here, its just for testing
	m_RHI->GetDeviceContext()->IASetInputLayout(m_StandardDiffuseShader->GetInputLayout());
	m_RHI->GetDeviceContext()->VSSetShader(m_StandardDiffuseShader->GetVertexShader(), NULL, 0);
	m_RHI->GetDeviceContext()->PSSetShader(m_StandardDiffuseShader->GetPixelShader(), NULL, 0);
	m_RHI->GetDeviceContext()->PSSetSamplers(0, 1, m_StandardDiffuseShader->GetAddressOfSamplerState());

	Fixture* fixture = g_AssetManager.GetFirstFixture();
	auto time = std::chrono::system_clock::now();
	for (int i = 0; i < fixture->m_SubMeshGeometry.size(); ++i) {
		auto& subMesh = fixture->m_SubMeshGeometry[i];
		using namespace DirectX::SimpleMath;
		Vector3 S = Vector3(0.01f, 0.01f, 0.01f);
		Quaternion R = Quaternion::Identity;
		// We want a rotation around the x axis which is the pitch
		//Quaternion R = Quaternion::CreateFromPitchYawRoll(-90.f, -180.f, 0.f); // Should move this rotation to occur when making the vertex buffers
		//Quaternion R = Quaternion::CreateFromPitchYawRoll(0.f, 0.f, time.time_since_epoch().count() % 360);
		//Quaternion R = Quaternion::CreateFromPitchYawRoll(-90.f, 0.f, 0.f);
		//Vector3 T = Vector3(0, 0, 400);
		Vector3 T = Vector3(0, 400, 0);

		Matrix localToWorldMatrix = Matrix::SRT(S, R, T);
		localToWorldMatrix = XMMatrixTranspose(localToWorldMatrix);
		Matrix worldToLocalMatrix = localToWorldMatrix.Invert();
		worldToLocalMatrix = XMMatrixTranspose(worldToLocalMatrix);

		void* drawBuffer = malloc(CB_DRAW_SIZE);
		void* drawBufferIndex1 = (void*)((char*)drawBuffer + sizeof(Matrix));
		memcpy(drawBuffer, &localToWorldMatrix, sizeof(localToWorldMatrix));
		memcpy(drawBufferIndex1, &worldToLocalMatrix, sizeof(worldToLocalMatrix));

		m_RHI->UpdateConstantBuffer(m_drawCBuffer.Get()->Get(), drawBuffer);
		free(drawBuffer);	

		UINT32 stride = sizeof(VertexType);
		UINT32 offset = 0;

		m_RHI->GetDeviceContext()->IASetVertexBuffers(0, 1, subMesh->GetVertexBuffer()->GetAddressOf(), &stride, &offset);
		m_RHI->GetDeviceContext()->IASetIndexBuffer(subMesh->GetIndexBuffer()->Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		m_RHI->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		std::string textureFileName = Fixture::GetSubMeshBaseTexture(fixture->m_SubMeshes[i]);
		Texture2D* texture = g_AssetManager.GetExternalTexture(textureFileName);
		if (texture != nullptr)
		{
			m_RHI->GetDeviceContext()->PSSetShaderResources(0, 1, texture->m_SRV.GetAddressOf());
		}

		m_RHI->GetDeviceContext()->DrawIndexed(subMesh->GetIndexBuffer()->m_IndexCount, 0, 0);
	}

	//{
	//	using namespace DirectX::SimpleMath;
	//	Vector3 S = Vector3(1, 1, 1);
	//	Quaternion R = Quaternion::Identity;
	//	Vector3 T = Vector3(0, 0, 5);

	//	Matrix localToWorldMatrix = Matrix::SRT(S, R, T);
	//	localToWorldMatrix = XMMatrixTranspose(localToWorldMatrix);
	//	Matrix worldToLocalMatrix = localToWorldMatrix.Invert();
	//	worldToLocalMatrix = XMMatrixTranspose(worldToLocalMatrix);

	//	void* drawBuffer = malloc(CB_DRAW_SIZE);
	//	void* drawBufferIndex1 = (void*)((char*)drawBuffer + sizeof(Matrix));
	//	memcpy(drawBuffer, &localToWorldMatrix, sizeof(localToWorldMatrix));
	//	memcpy(drawBufferIndex1, &worldToLocalMatrix, sizeof(worldToLocalMatrix));

	//	m_RHI->UpdateConstantBuffer(m_drawCBuffer.Get()->Get(), drawBuffer);
	//	free(drawBuffer);	

	//	UINT32 stride = sizeof(VertexType);
	//	UINT32 offset = 0;

	//	ComPtr<Mesh> triangle = new Mesh();
	//	triangle->InitializeAsTriangle();

	//	m_RHI->GetDeviceContext()->IASetVertexBuffers(0, 1, triangle->GetVertexBuffer()->GetAddressOf(), &stride, &offset);
	//	m_RHI->GetDeviceContext()->IASetIndexBuffer(triangle->GetIndexBuffer()->Get(), DXGI_FORMAT_R32_UINT, 0);

	//	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	//	m_RHI->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	m_RHI->GetDeviceContext()->DrawIndexed(triangle->GetIndexBuffer()->m_IndexCount, 0, 0);
	//}
	

	//RENDER
	m_RHI->Render();

	m_RHI->EndFrame();

	return true;
}

bool RenderEngine::CreateConstantBuffers()
{
	HRESULT hres;
	m_applicationCBuffer = new D3D11ConstantBuffer();
	hres = m_applicationCBuffer->Initialize(m_RHI->GetDevice(), CB_APPLICATION_SIZE);
	if (FAILED(hres))
	{
		return false;
	}

	m_frameCBuffer = new D3D11ConstantBuffer();
	hres = m_frameCBuffer->Initialize(m_RHI->GetDevice(), CB_FRAME_SIZE);
	if (FAILED(hres))
	{
		return false;
	}

	m_drawCBuffer = new D3D11ConstantBuffer();
	hres = m_drawCBuffer->Initialize(m_RHI->GetDevice(), CB_DRAW_SIZE);
	if (FAILED(hres))
	{
		return false;
	}

	// SHOULD NOT BE HERE, FOR QUICK TESTING RIGHT NOW
	m_globalsCBuffer = new D3D11ConstantBuffer();
	hres = m_globalsCBuffer->Initialize(m_RHI->GetDevice(), 16);
	if (FAILED(hres))
	{
		return false;
	}

	return true;
}

void RenderEngine::BindConstantBuffers()
{
	ID3D11DeviceContext* deviceContext = m_RHI->GetDeviceContext();
	deviceContext->VSSetConstantBuffers(CB_DRAW, 1, m_drawCBuffer->GetAddressOf());
	deviceContext->PSSetConstantBuffers(CB_DRAW, 1, m_drawCBuffer->GetAddressOf());
	deviceContext->VSSetConstantBuffers(CB_FRAME, 1, m_frameCBuffer->GetAddressOf());
	deviceContext->PSSetConstantBuffers(CB_FRAME, 1, m_frameCBuffer->GetAddressOf());
	deviceContext->VSSetConstantBuffers(CB_APPLICATION, 1, m_applicationCBuffer->GetAddressOf());
	deviceContext->PSSetConstantBuffers(CB_APPLICATION, 1, m_applicationCBuffer->GetAddressOf());

	// SHOULD NOT BE HERE, FOR QUICK TESTING RIGHT NOW
	deviceContext->VSSetConstantBuffers(CB_GLOBALS, 1, m_globalsCBuffer->GetAddressOf());
	deviceContext->PSSetConstantBuffers(CB_GLOBALS, 1, m_globalsCBuffer->GetAddressOf());
}

