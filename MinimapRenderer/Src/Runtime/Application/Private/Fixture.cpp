#include "Fixture.h"

#include "niflib.h"
#include "obj/NiObject.h"
#include "obj/NiNode.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTextureProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiImage.h"


bool Fixture::Init() {
	DebugLog("ss", "Opening NIF: ", m_FilePath.c_str());

	UINT32 nifVer = Niflib::GetNifVersion(m_FilePath);
	if (Niflib::IsSupportedVersion(nifVer) == false) {
		DebugLog("s", "NIF file not supported.");
		return false;
	}
	else if (nifVer == Niflib::VER_INVALID) {
		DebugLog("s", "Not a NIF file.");
		return false;
	}
	else {
		std::string ver = Niflib::FormatVersionString(nifVer);
		DebugLog("s", ver.c_str());

		Niflib::NiObjectRef rootObject = Niflib::ReadNifTree(m_FilePath);
		Niflib::NiNodeRef rootNode;
		rootNode = Niflib::DynamicCast<Niflib::NiNode>(rootObject);

		std::stack<Niflib::NiNodeRef> nifStack;
		if (rootNode != nullptr) {
			DebugLog("s", "Successful reading of nif tree into object.");
			//DebugLog("s", rootNode->asString().c_str());
			nifStack.push(rootNode);
		}

		while (!nifStack.empty()) {
			Niflib::NiNodeRef node = nifStack.top();
			nifStack.pop();
			auto children = node->GetChildren();
			for (auto& child : children) {
				if (!child->IsDerivedType(Niflib::NiGeometry::TYPE)) {
					Niflib::NiNodeRef newNode = Niflib::DynamicCast<Niflib::NiNode>(child);
					if (newNode != nullptr)
						nifStack.push(newNode);
				}
				else {
					// Only add SubMeshes that we can render using the MythicGeneral material
					auto subMesh = Niflib::DynamicCast<Niflib::NiGeometry>(child);
					if (IsRenderable(subMesh)) {
						m_SubMeshes.emplace_back(subMesh);
					}
				}
			}
		}

		if (m_SubMeshes.empty()) {
			DebugLog("s", "No submeshes.");
		}

		// This needs to be cleaned up, right now I decided to base the entire application on the NIF's coordinate system
		// But it would be better to bring everything into RH DirectX. I should be able to apply a change of axis at the end to fix the normals and vertices. 
		for (auto& nifGeomRef : m_SubMeshes) {
			std::vector<Niflib::Vector3> vertices = Fixture::GetSubMeshVertices(nifGeomRef);
			std::vector<Niflib::Vector3> normals = Fixture::GetSubmeshNormals(nifGeomRef);
			std::vector<Niflib::TexCoord> uvs = Fixture::GetSubMeshUVs(nifGeomRef, 0); // UV set 0 is definitely used to sample the diffuse texture

			if (vertices.size() != normals.size() || vertices.size() != uvs.size()) {
				DebugLog("s", "Number of vertices, normals and uvs is not equal for this submesh");
				return false; // This is valid as the vector is filled ONLY with valid submeshes otherwise error
			}
	
			std::vector<UINT32> indexData = GetVertexIndices(nifGeomRef);
			std::vector<VertexType> vertexData(vertices.size());
			{
				// I think everything needs to be converted to the proper coordinate system first
				//z is actually y, y is actually negative z, x is actually negative x - so <-x, z, -y> is the vector we need for translation or position
				//now we need to take care of rotation, how do we fix that? We can get the euler angles and then change what they represent a rotation around

				using namespace DirectX::SimpleMath;
				Niflib::Matrix44 worldTransform = nifGeomRef->GetWorldTransform();
				//Niflib::Vector3 worldTranslation = worldTransform.GetTranslation();
				//Niflib::Quaternion worldRotation = worldTransform.GetRotation().AsQuaternion();
				//float worldScale = worldTransform.GetScale();

				////DebugLog("sfsfsfs", "<", worldRotation.AsEulerYawPitchRoll()[0], ", ", worldRotation.AsEulerYawPitchRoll()[1], ", ", worldRotation.AsEulerYawPitchRoll()[2], ">");

				//Vector3 S = Vector3(worldScale, worldScale, worldScale);
				//float rad2Deg = PI * 2.f / 360.f;
				///*Quaternion R = Quaternion::CreateFromPitchYawRoll(worldRotation.AsEulerYawPitchRoll()[1] * rad2Deg, 
				//												  worldRotation.AsEulerYawPitchRoll()[0] * rad2Deg, 
				//												  worldRotation.AsEulerYawPitchRoll()[2] * rad2Deg);*/
				//Vector3 T = Vector3(worldTranslation.x, worldTranslation.y, worldTranslation.z);

				//Matrix localToParent = Matrix::SRT(S, R, T);

				//Quaternion coordinateChange = Quaternion::CreateFromPitchYawRoll TODO

				/*Niflib::Matrix33 normalWorldTransform(worldTransform[0][0], worldTransform[0][1], worldTransform[0][2],
					worldTransform[1][0], worldTransform[1][1], worldTransform[1][2],
					worldTransform[2][0], worldTransform[2][1], worldTransform[2][2]);*/

				Niflib::Matrix44 normalWorldTransform = worldTransform.Inverse();

				// I need to rotate everything by 90 degrees around the x axis then negate the x-axis TODO
				for (int i = 0; i < vertexData.size(); ++i) {
					// Apply heirarchy transforms - COULD IT BE THAT TRANSFORM IS APPLYING MATRIX MULTIPLICATION FROM THE WRONG SIDE?! TODO
					//Vector4 vertex = DirectX::SimpleMath::Vector4::Transform(Vector4(vertices[i].x, vertices[i].y, vertices[i].z, 1.f), localToParent);
					//Vector3 normal = DirectX::SimpleMath::Vector3::TransformNormal(Vector3(normals[i].x, normals[i].y, normals[i].z), localToParent);

					Niflib::Vector3 n = Niflib::Vector3(normals[i].x, normals[i].y, normals[i].z);

					Niflib::Vector3 normal;
					/*normal.x = n.x * normalWorldTransform[0][0] + n.y * normalWorldTransform[1][0] + n.z * normalWorldTransform[2][0];
					normal.y = n.x * normalWorldTransform[0][1] + n.y * normalWorldTransform[1][1] + n.z * normalWorldTransform[2][1];
					normal.z = n.x * normalWorldTransform[0][2] + n.y * normalWorldTransform[1][2] + n.z * normalWorldTransform[2][2];*/

					normal.x = n.x * normalWorldTransform[0][0] + n.y * normalWorldTransform[0][1] + n.z * normalWorldTransform[0][2];
					normal.y = n.x * normalWorldTransform[1][0] + n.y * normalWorldTransform[1][1] + n.z * normalWorldTransform[1][2];
					normal.z = n.x * normalWorldTransform[2][0] + n.y * normalWorldTransform[2][1] + n.z * normalWorldTransform[2][2];

					// TEST
					Niflib::Vector3 vertex = Niflib::Vector3(vertices[i].x, vertices[i].y, vertices[i].z);
					vertex = worldTransform * vertex;

					vertexData[i] = { 
										Vector4(vertex.x, vertex.y, vertex.z, 1.f),
										Vector4(normal.x, normal.y, normal.z, 1.f),
										Vector4(uvs[i].u, uvs[i].v, 1.f, 1.f) 
									};
				}
			}
			DebugLog("si", "Number of vertices: ", vertexData.size());
			DebugLog("si", "Number of indices: ", indexData.size());

			m_SubMeshGeometry.emplace_back(new Mesh());
			if (m_SubMeshGeometry.back()->Initialize(vertexData, indexData)) {
				DebugLog("s", "Mesh successfully initialized.");
			}
			else {
				DebugLog("s", "Failed to create Vertex and Index buffers.");
				return false;
			}
			
		}

		if (m_SubMeshes.empty()) {
			DebugLog("s", "There is nothing to render for this asset.");
			return false;
		}
		else {
			return true;
		}
	}
}

bool Fixture::IsRenderable(const Niflib::NiGeometryRef& subMesh) {
	if (GetSubMeshBaseTexture(subMesh) == "") {
		DebugLog("s", "Submesh didnt have a base map.");
	}

	Niflib::NiGeometryDataRef geomData = subMesh->GetData();
	if (geomData->GetUVSetCount() == 0 || GetSubMeshBaseTexture(subMesh) == "") {
	//if (geomData->GetUVSetCount() == 0) {
		return false;
	}
	else {
		return true;
	}
}

std::string Fixture::GetSubMeshBaseTexture(const Niflib::NiGeometryRef& subMesh) {
	std::string res = "";

	auto properties = subMesh->GetProperties();
	for (auto property : properties) {
		if (property->IsDerivedType(Niflib::NiTexturingProperty::TYPE)) {
			Niflib::NiTexturingPropertyRef texturingProperty = Niflib::DynamicCast<Niflib::NiTexturingProperty>(property);
			if (texturingProperty != nullptr) {
				if (texturingProperty->HasTexture(Niflib::TexType::BASE_MAP)) {
					Niflib::TexDesc& texDesc = texturingProperty->GetTexture(Niflib::TexType::BASE_MAP);
					auto texSource = texDesc.source;
					//if (texSource->IsTextureExternal()) {
						res = texSource->GetTextureFileName();
					//}
				}
			}
		}
	}

	// Set all characters to lower case to sanitize the string
	if (!res.empty()) {
		std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	}

	return res;
}

std::vector<Niflib::Vector3> Fixture::GetSubMeshVertices(const Niflib::NiGeometryRef& subMesh) {
	Niflib::NiGeometryDataRef geomData = subMesh->GetData();
	return geomData->GetVertices();
}

std::vector<Niflib::TexCoord> Fixture::GetSubMeshUVs(const Niflib::NiGeometryRef& subMesh, const int uvSetIndex) {
	Niflib::NiGeometryDataRef geomData = subMesh->GetData();
	if (uvSetIndex >= geomData->GetUVSetCount()) {
		return std::vector<Niflib::TexCoord>();
	}
	else {
		return geomData->GetUVSet(uvSetIndex);
	}
}

std::vector<Niflib::Vector3> Fixture::GetSubmeshNormals(const Niflib::NiGeometryRef& subMesh) {
	Niflib::NiGeometryDataRef geomData = subMesh->GetData();
	return geomData->GetNormals();
}

std::vector<UINT32> Fixture::GetVertexIndices(const Niflib::NiGeometryRef& subMesh) {
	Niflib::NiGeometryDataRef geomData = subMesh->GetData();
	Niflib::NiTriShapeDataRef triShapeData = Niflib::DynamicCast<Niflib::NiTriShapeData>(geomData);
	std::vector<Niflib::Triangle> triangles = triShapeData->GetTriangles();

	std::vector<UINT32> indices(triangles.size() * 3);
	int index = 0;
	for (int i = 0; i < triangles.size(); ++i) {
		indices[index++] = triangles[i].v1;
		indices[index++] = triangles[i].v2;
		indices[index++] = triangles[i].v3;
	}
	return indices;
}