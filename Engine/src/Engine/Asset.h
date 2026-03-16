#pragma once
#include "Engine/Engine.h"
#include "Engine/YamlExtensions.h"

namespace PH::Engine::Assets {

	enum struct AssetStatus {
		NOT_LOADED,
		LOADED,
		LOADING
	};

	enum class AssetType {
		NONE,
		TEXTURE_IMAGE,
		MATERIAL,
		MATERIAL_INSTANCE,
		MESH,
		SCENE,
		SCRIPT,
		UNDEFINED
	};

	struct Asset {

		static constexpr const char* getExtension() {
			return "";
		}

		static constexpr AssetType getType() {
			return AssetType::UNDEFINED;
		}

		UUID id;
		AssetType type;

		bool32 internsource;
		Engine::String filepath;
		Engine::ArrayList<String> references;

		volatile AssetStatus status;

	friend class Library;
	private:

		//runs when the asset is included into the library
		static bool32 include(YAML::Node& root, Library* lib, Asset* instance) {
			return true;
		}

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, Asset* instance) {
			WARN << "you are tring to deserialize an asset without a specified serialize method!\n";
			return;
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, Asset* result, const char* path) {
			WARN << "you are tring to deserialize an asset without a specified deserialize method!\n";
			return false;
		}

		static bool32 loadFromSource(const char* sourcepath, Asset* result) {
			WARN << "you are trying to load an asset from source that does not have a loadfrom source method\n";
		}
	};

}