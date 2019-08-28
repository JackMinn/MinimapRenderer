#include "Windows/WindowsApplication.h"
#include "Misc/Utilities.h"

#include "niflib.h"
#include "obj/NiObject.h"
#include "obj/NiNode.h"
#include "obj/NiAVObject.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTextureProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiImage.h"

#include "Engine/RenderEngine.h"
#include "Input.h"
#include "AssetManager.h"
#include "Fixture.h"

#include <d3d11.h>
#include "SimpleMath.h"

LPCWSTR g_ApplicationName = L"NIF Scene Renderer - DirectX11";
RenderEngine g_RenderEngine;
AssetManager g_AssetManager;

INT32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pScmdline, int iCmdshow)
{
	bool result;
	WindowsApplication mainApplication;
	mainApplication.CreateApplication(hInstance, g_ApplicationName, nullptr);

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

	LONG clientWidth, clientHeight;
	mainApplication.GetClientDimensions(clientWidth, clientHeight);

	DebugLog("sisi", "Client Width is: ", clientWidth, " Client Height is: ", clientHeight);

	result = g_RenderEngine.Initialize(clientWidth, clientHeight, mainApplication.GetWindowHandle());
	if (!result)
	{
		return 0;
	}

	//fi.0.0.dw_rvr_fortress.nif
	std::string fileName(pScmdline);

	g_AssetManager.Initialize(mainApplication.GetWindowHandle(), "zones/TestZone/", "assetdb/fixtures/", "assetdb/textures/");
	//g_AssetManager.AddFixture(fileName);
	g_AssetManager.LoadAllFixtures();
	g_AssetManager.LoadAllFixtureInstances();
	
	//Fixture* fixture = g_AssetManager.GetFixture(fileName);

	//if (fixture == nullptr) {
	//	return 0;
	//}
	//else {
	//	DebugLog("si", "Number of renderable assets is: ", fixture->GetNumSubMeshes());
	//	DebugLog("si", "Number of external textures loaded is: ", g_AssetManager.GetNumExternalTextures());

		//// Remember, aside from the instance data transform, each fixture has its own world transform
		//for (auto& geomRef : fixture->m_SubMeshes) {
		//	auto worldTransform = geomRef->GetWorldTransform();
		//	auto worldTranslation = worldTransform.GetTranslation();
		//	//DebugLog("ssfsfsfs", "Translation is: ", "<", worldTranslation.x, ", ", worldTranslation.y, ", ", worldTranslation.z, ">");

		//	Niflib::NiGeometryDataRef geomData = geomRef->GetData();
		//	std::vector<Niflib::Vector3> vertices = geomData->GetVertices();
		//}
	//}

	mainApplication.ProcessMessagesLoop();

	// Shutdown and release the system object.
	g_RenderEngine.Shutdown();
	mainApplication.ShutdownApplication();

	return 0;
}

