#pragma once

#include <Engine/Engine.h>
#include <Engine/AssetDescription.h>
#include <Engine/AssetLibrary.h>

#define PH_ASSET_TEXTURE 15876590110196968583

namespace PH::Engine {

	struct Texture : public Engine::AssetBase {
		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_TEXTURE;
		}

		Platform::GFX::Texture texture;
		sizeptr width, height;

		Engine::String source;

		Platform::GFX::Format format;
	};

	PH_ENGINE_API Texture createTexture(Base::Array<uint8> bitmap, uint32 width, uint32 height, PH::Platform::GFX::Format format);
	PH_ENGINE_API Texture createTexture(const Platform::FileBuffer& f, PH::Platform::GFX::Format format, PH::Platform::GFX::ImageUsageFlags imageusage);
	PH_ENGINE_API Texture createTexture(const char* sourcepath, Platform::GFX::Format format);
	PH_ENGINE_API Texture createTexture(const char* texture);
	Engine::AssetDescription createTextureAssetDescription();
}