#pragma once

#include "Engine/Texture2D.h"

class FixtureInstance;
class Fixture;
class Mesh;

class AssetManager {
public:
	AssetManager();
	~AssetManager();
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(AssetManager);

	void Initialize(HWND windowHandle, const std::string& zoneDirectory, const std::string& fixtureDir, const std::string& textureDir);
	void LoadAllFixtures();
	void LoadAllFixtureInstances();
	bool AddFixture(const std::string& fileName);
	Fixture* GetFixture(const std::string& fileName);
	UINT32 GetNumExternalTextures() const { return m_ExternalTextures.size(); }
	Texture2D* GetExternalTexture(const std::string& file) {
		if (m_ExternalTextures.count(file) > 0) {
			return m_ExternalTextures[file].Get();
		}
		else {
			return nullptr;
		}
	}

	const std::vector<FixtureInstance>& GetFixtureInstances() const { return m_FixtureInstances; }
	// TEMP FOR TESTING
	Fixture* GetFirstFixture() const { return m_Fixtures.begin()->second; }
	//Fixture* GetFirstFixture() const { return m_Fixtures.at("fi.0.0.dw_rvr_fortress.nif"); }

private:
	HWND m_WindowHandle;
	std::string m_ZoneDirectory;
	std::string m_FixtureDirectory;
	std::string m_TextureDirectory;
	std::unordered_map<std::string, Fixture*> m_Fixtures;
	//std::unordered_map<std::string, std::vector<ComPtr<Mesh>>> m_FixtureSubMeshes;
	std::unordered_map<std::string, ComPtr<Texture2D>> m_ExternalTextures;
	std::vector<FixtureInstance> m_FixtureInstances;
};
