#pragma once
#include "Engine/Asset.h"
#include "Engine/assets/Material.h"

namespace PH::Engine::Assets {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 UV;
	};

#define CUBEMAP_FRONT	0
#define CUBEMAP_BACK	1
#define CUBEMAP_UP		2
#define CUBEMAP_DOWN	0
#define CUBEMAP_RIGHT	0
#define CUBEMAP_LEFT	0

	struct Mesh;

	bool32 serializeMesh(YAML::Emitter& out, Mesh* mesh);
	bool32 serializeMesh(const char* filepath, Mesh* mesh);
	bool32 deserializeMesh(YAML::Node& root, Library* lib, Mesh* mesh, const char* path);
	bool32 loadMeshsource(const char* filepath, Mesh* mesh);

	struct Mesh : public Asset {

		static constexpr const char* getExtension() {
			return ".phmesh";
		}

		static constexpr AssetType getType() {
			return AssetType::MESH;
		}

		bool32 Ready() {
			return (status == AssetStatus::LOADED);
		}

		Platform::GFX::Buffer vertexbuffer;
		Platform::GFX::Buffer indexbuffer;

		Engine::String sourcepath;

		ArrayList<Assets::Vertex> localvb;
		ArrayList<uint32> localib;

		uint32 submeshcount;

		struct SubMesh {
			String name;

			sizeptr offset;
			sizeptr count;
		};

		ArrayList<SubMesh> submeshes;
		ArrayList<UUID> defaultmaterials;

		friend class Assets::Library;
	private:

		//runs when an asset is included in the libary
		static bool32 include(YAML::Node& root, Library* lib, Mesh* instance);

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, Mesh* instance) {
			serializeMesh(out, instance);
			return;
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, Mesh* result, const char* path) {
			return deserializeMesh(root, lib, result, path);
		}

		static bool32 loadFromSource(const char* sourcepath, Mesh* result) {
			return loadMeshsource(sourcepath, result);
		}

	};

	//loads a mesh and creates materials for it base on the mastermaterial
	bool32 importMesh(const char* filepath, const char* targetdir, Mesh* mesh, Material* mastermaterial, Library* lib);
	Mesh createMeshFromData(Base::Array<Assets::Vertex> vertices, Base::Array<uint32> indices, bool32 copylocal);
}
