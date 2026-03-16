#pragma once
#include "../Asset.h"
#include "../Engine.h"

namespace PH::Engine::Assets {

	struct TextureImage;
	struct CubeMap;

	bool32 serializeTextureImage(YAML::Emitter& out, TextureImage* image);
	bool32 deserializeTextureImage(const char* path, TextureImage* image);
	bool32 deserializeTextureImage(YAML::Node& root, TextureImage* result, const char* path);
	bool32 loadTextureSource(const char* filepath, TextureImage* image);

	struct TextureImage : public Asset {

		static constexpr const char* getExtension() {
			return ".phtex";
		}

		static constexpr AssetType getType() {
			return AssetType::TEXTURE_IMAGE;
		}

		Engine::String sourcepath;

		Platform::GFX::Texture base;
		Platform::GFX::Format format;

		//serializes the asset into the yaml emitter
		static void serialize(YAML::Emitter& out, TextureImage* instance) {
			serializeTextureImage(out, instance);
			return;
		}

		//deserializes the asset, the path parameter is only there for debug messages...
		static bool32 deserialize(YAML::Node& root, Library* lib, TextureImage* result, const char* path) {
			return deserializeTextureImage(root, result, path);
		}

		static bool32 loadFromSource(const char* sourcepath, TextureImage* result) {
			return loadTextureSource(sourcepath, result);
		}

	};

	struct CubeMap : Asset {
		Platform::GFX::Texture base;
		Engine::String sources[6];
	};

	bool32 loadCubeMapSource(const char** filepaths, CubeMap* map);

	bool32 deserializeTextureImage(const char* path, TextureImage* image);
}