#include "AssetManager.h"
#include "Fixture.h"
#include "Engine/RenderEngine.h"

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

void AssetManager::Initialize(const std::string& fixtureDir, const std::string& textureDir)
{
	m_FixtureDirectory = fixtureDir;
	m_TextureDirectory = textureDir;
}

// Might be better to have a more global way of accessing the window handle,
// as other functions might later want to raise a MessageBox error, but we can
// deal with this in a future refactor
bool AssetManager::AddFixture(const std::string& fileName, HWND hwnd) {
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
					MessageBox(hwnd, errorW.c_str(), L"Error", MB_OK);
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