#pragma once

#include "obj/NiTriShape.h"

#include "Engine/Mesh.h"

class Fixture {
public:
	int GetNumSubMeshes() const { return m_SubMeshes.size(); }
	Niflib::Matrix44 GetWorldTransform(const Niflib::NiGeometryRef& subMesh, Niflib::Matrix44& out) const { out = subMesh->GetWorldTransform(); }

	static bool IsRenderable(const Niflib::NiGeometryRef& subMesh);
	static std::string GetSubMeshBaseTexture (const Niflib::NiGeometryRef& subMesh);
	static std::vector<Niflib::Vector3> GetSubmeshNormals(const Niflib::NiGeometryRef& subMesh);
	static std::vector<Niflib::TexCoord> GetSubMeshUVs(const Niflib::NiGeometryRef& subMesh, const int uvSetIndex);
	static std::vector<Niflib::Vector3> GetSubMeshVertices(const Niflib::NiGeometryRef& subMesh);
	static std::vector<UINT32> GetVertexIndices(const Niflib::NiGeometryRef& subMesh);

private:
	Fixture(const std::string& filePath) : m_FilePath(filePath) {}
	~Fixture() {}
	NO_COPY_CONSTRUCT_OR_COPY_ASSIGN(Fixture);

	bool Init();

	friend class AssetManager;

public:
	std::vector<Niflib::NiGeometryRef> m_SubMeshes;
	std::vector<ComPtr<Mesh>> m_SubMeshGeometry;

private:
	std::string m_FilePath;
};
