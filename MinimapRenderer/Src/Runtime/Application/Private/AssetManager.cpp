#include "AssetManager.h"
#include "Fixture.h"
#include "FixtureInstance.h"
#include "Engine/RenderEngine.h"
#include "rapidcsv.h"
#include <string.h>
#include <locale.h>

#include <d3d11.h>
#include "SimpleMath.h"

extern RenderEngine g_RenderEngine;

AssetManager::AssetManager()
{}

AssetManager::~AssetManager() {
	auto it = m_Fixtures.begin();
	while (it != m_Fixtures.end()) {
		delete it->second;
		it = m_Fixtures.erase(it);
	}
	assert(m_Fixtures.size() == 0);

	// Might not be needed, as ComPtrs will clean themselves up when the map self deletes
	auto it2 = m_ExternalTextures.begin();
	while (it2 != m_ExternalTextures.end()) {
		it2 = m_ExternalTextures.erase(it2);
	}
	assert(m_ExternalTextures.size() == 0);
}

void AssetManager::Initialize(HWND windowHandle, const std::string& zoneDirectory, const std::string& fixtureDir, const std::string& textureDir)
{
	m_WindowHandle = windowHandle;
	m_ZoneDirectory = zoneDirectory;
	m_FixtureDirectory = fixtureDir;
	m_TextureDirectory = textureDir;
}

void AssetManager::LoadAllFixtures()
{
	rapidcsv::Document doc(m_ZoneDirectory + "nifs.csv");

	std::vector <std::string> fixtures = doc.GetColumn<std::string>(1);
	DebugLog("su", "Number of fixtures read: ", (UINT32)fixtures.size());
	UINT32 successfullyLoaded = 0;
	for (int i = 1; i < fixtures.size(); ++i) {
		//DebugLog("s", fixtures[i].c_str());
		if (AddFixture("fi.0.0." + fixtures[i])) {
			successfullyLoaded++;
		}
		else {
			DebugLog("s", "Failed to load this fixture.");
		}
	}
	DebugLog("su", "Number of fixtures successfully loaded: ", successfullyLoaded);
}

void AssetManager::LoadAllFixtureInstances()
{
	rapidcsv::Document doc(m_ZoneDirectory + "fixtures.csv");
	std::vector <std::string> names = doc.GetColumn<std::string>(1);
	std::vector<string> x = doc.GetColumn<string>(2);
	std::vector <string> y = doc.GetColumn <string>(3);
	std::vector <string> z = doc.GetColumn <string>(4);
	std::vector <string> scale = doc.GetColumn<string>(6);

	std::vector <string> angle = doc.GetColumn<string>(14);
	std::vector <string> rotX = doc.GetColumn<string>(15);
	std::vector <string> rotY = doc.GetColumn<string>(16);
	std::vector <string> rotZ = doc.GetColumn<string>(17);

	for (int i = 1; i < names.size(); ++i) {
		using namespace DirectX::SimpleMath;

		string name = "fi.0.0." + names[i].substr(0, names[i].find(' ')) + ".nif";
		DebugLog("s", name.c_str());

		RemoveWhiteSpaces(x[i]);
		RemoveWhiteSpaces(y[i]);
		RemoveWhiteSpaces(z[i]);
		//positions[i - 1] = Niflib::Vector3(std::stof(x[i]), std::stof(y[i]), std::stof(z[i]));
		Vector3 position = Vector3(std::stof(x[i]), std::stof(y[i]), std::stof(z[i]));
		
		RemoveWhiteSpaces(scale[i]);
		Vector3 size = Vector3(std::stof(scale[i]), std::stof(scale[i]), std::stof(scale[i]));
		
		RemoveWhiteSpaces(angle[i]);
		RemoveWhiteSpaces(rotX[i]);
		RemoveWhiteSpaces(rotY[i]);
		RemoveWhiteSpaces(rotZ[i]);

		Vector3 axis = Vector3(std::stof(rotX[i]), std::stof(rotY[i]), std::stof(rotZ[i]));
		Quaternion rot = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(axis, std::stof(angle[i]));

		m_FixtureInstances.emplace_back(FixtureInstance(name, position, rot, size));
	}

	DebugLog("su", "Number of instances: ", (UINT32)m_FixtureInstances.size());
}

// Might be better to have a more global way of accessing the window handle,
// as other functions might later want to raise a MessageBox error, but we can
// deal with this in a future refactor
bool AssetManager::AddFixture(const std::string& fileName) {
	if (m_Fixtures.count(fileName) > 0) {
		DebugLog("s", "Fixture has already been initialized.");
		return true;
	}

	std::string filePath = m_FixtureDirectory + fileName;
	Fixture* fixture = new Fixture(filePath);
	bool res = fixture->Init();
	if (res == true) {
		m_Fixtures[fileName] = fixture;
		for (auto& geomRef : fixture->m_SubMeshes) {
			std::string textureName = Fixture::GetSubMeshBaseTexture(geomRef);

			if (m_ExternalTextures.count(textureName) == 0 && !textureName.empty()) {
				auto& texture2DPtr = m_ExternalTextures[textureName];
				texture2DPtr = new Texture2D();
				// The below code needs to be dynamic
				std::string prefix = fileName.substr(0, 7);
				bool result = g_RenderEngine.CreateTexture2DFromFile(m_TextureDirectory, prefix, textureName, texture2DPtr.Get());

				if (!result) {
					std::string error = "Could not find texture " + textureName;
					std::wstring errorW(error.begin(), error.end());
					MessageBox(m_WindowHandle, errorW.c_str(), L"Error", MB_OK);
					m_ExternalTextures.erase(textureName);
				}
				else {
					DebugLog("ss", textureName.c_str(), " added to Asset DB.");
				}
			}
		}
	}
	else {
		delete fixture;
	}

	return res;
}

Fixture* AssetManager::GetFixture(const std::string& fileName) {
	if (m_Fixtures.count(fileName) > 0) {
		return m_Fixtures[fileName];
	}
	else {
		return nullptr;
	}
}