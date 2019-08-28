#include "Engine/RenderEngine.h"

#include <chrono>

#include "AssetManager.h"
#include "Fixture.h"
#include "FixtureInstance.h"
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
	// BUT FOR SIMPLICITY WILL STAY HERE TO EASILY SWITCH BETWEEN PERSPECTIVE AND ORTHOGRAPHIC LATER
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

	// LOOP OVER ASSET MANAGER TO DO BINDINGS OF Model Matrix and Texture 

	// For the time being we use a single diffuse shader for everything, down the road we would like to use materials
	// which are bound to certain instances/fixtures
	DirectX::SimpleMath::Vector4 globals = DirectX::SimpleMath::Vector4::One;
	m_RHI->UpdateConstantBuffer(m_globalsCBuffer.Get()->Get(), (void*)(&globals));

	m_RHI->GetDeviceContext()->IASetInputLayout(m_StandardDiffuseShader->GetInputLayout());
	m_RHI->GetDeviceContext()->VSSetShader(m_StandardDiffuseShader->GetVertexShader(), NULL, 0);
	m_RHI->GetDeviceContext()->PSSetShader(m_StandardDiffuseShader->GetPixelShader(), NULL, 0);
	m_RHI->GetDeviceContext()->PSSetSamplers(0, 1, m_StandardDiffuseShader->GetAddressOfSamplerState());


	const std::vector<FixtureInstance>& instances = g_AssetManager.GetFixtureInstances();
	for (int i = 0; i < instances.size(); ++i) {
		using namespace DirectX::SimpleMath;
		Fixture* fixture = g_AssetManager.GetFixture(instances[i].GetFixtureName());
		Matrix localToWorldMatrix = instances[i].m_Transform.GetLocalToWorldMatrixGPU();
		Matrix worldToLocalMatrix = instances[i].m_Transform.GetWorldToLocalMatrixGPU();

		if (!fixture)
			continue;

		for (int i = 0; i < fixture->m_SubMeshGeometry.size(); ++i) {
			auto& subMesh = fixture->m_SubMeshGeometry[i];
			/*Vector3 S = Vector3(0.01f, 0.01f, 0.01f);
			Quaternion R = Quaternion::Identity;
			Vector3 T = Vector3(0, 400, 0);

			Matrix localToWorldMatrix = Matrix::SRT(S, R, T);
			localToWorldMatrix = XMMatrixTranspose(localToWorldMatrix);
			Matrix worldToLocalMatrix = localToWorldMatrix.Invert();
			worldToLocalMatrix = XMMatrixTranspose(worldToLocalMatrix);*/

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

		//RENDER
		m_RHI->Render(); // Does nothing right now, eventually all the directx code above should occur in this function and not here
	}

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
	// Later on materials should have their own constant buffers that stay on video memory
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

