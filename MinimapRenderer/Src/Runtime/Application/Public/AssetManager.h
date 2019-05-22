#pragma once

#include "Engine/Texture2D.h"

class Fixture;
class Mesh;

class AssetManager {
public:
	AssetManager();
	~AssetManager();
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(AssetManager);

	void Initialize(const std::string& fixtureDir, const std::string& textureDir);
	bool AddFixture(const std::string& fileName, HWND hwnd);
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

	// TEMP FOR TESTING
	Fixture* GetFirstFixture() const { return m_Fixtures.begin()->second; }

private:
	std::string m_FixtureDirectory;
	std::string m_TextureDirectory;
	std::unordered_map<std::string, Fixture*> m_Fixtures;
	//std::unordered_map<std::string, std::vector<ComPtr<Mesh>>> m_FixtureSubMeshes;
	std::unordered_map<std::string, ComPtr<Texture2D>> m_ExternalTextures;
};
