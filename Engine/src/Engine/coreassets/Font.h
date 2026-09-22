#pragma once
#include <Platform/platformAPI.h>
#include <Engine/AssetDescription.h>
#include <stb/stb_truetype.h>

namespace PH::Engine {

	struct Font : public AssetBase {

		Platform::GFX::Texture atlas;
		stbtt_bakedchar cdata_cpu[96];
		Platform::GFX::DescriptorSet cdata;
		Platform::GFX::Buffer cdatabuffer;

		uint32 bitmapwidth;
		real32 pixelheight;

		static Platform::GFX::DescriptorSetLayout descriptorsetlayout;
	};

	void serializeFont(const char* filepath, Font* font);
	void deserializeFont(const char* filepath, Font* font);
	Font createFont(const char* path);

	AssetDescription createFontDescription();

	inline const char* getFontExtension() {
		return ".lcfont";
	}

}