#pragma once

#include <LabCore/LabCoreAPI.h>

#define PH_ASSET_TEXTURE 15876590110196968583

namespace PH::LabCore3D {

	struct Texture : public Engine::AssetBase {
		static Engine::AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_TEXTURE;
		}

		Platform::GFX::Texture texture;
		sizeptr width, height;

		Platform::GFX::Format format;
	};

	Texture createTexture(const char* sourcepath, Platform::GFX::Format format);
	Texture createTexture(const char* texture);
	Engine::AssetDescription createTextureAssetDescription();
}