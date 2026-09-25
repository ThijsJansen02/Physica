#pragma once
#include <Platform/platformAPI.h>
#include <Engine/AssetDescription.h>
#include <stb/stb_truetype.h>

#include <Engine/Rendering.h>

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
	Font createFont(const char* ttfpath, uint32 bitmapwidth, real32 pixelheight);

	AssetDescription createFontDescription();

	void drawText(Font* font, const char* text, glm::vec2 position, real32 scale, const glm::vec4& color, Engine::Renderer2D::Context* context);

	inline const char* getFontExtension() {
		return ".lcfont";
	}

}