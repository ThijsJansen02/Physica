#pragma once
#include <Platform/platformAPI.h>
#include <Engine/AssetDescription.h>

#include <Engine/coreassets/Texture.h>
#include <Engine/Rendering.h>
#include <Engine/RenderPrimitives.h>
#include <Engine/GPUBuffer.h>

namespace PH::Engine {

//random hardcoded UUID for the font asset description
#define PH_ASSET_FONT 8122762271555803

	typedef uint32 unicode;
	
	struct Glyph {
		unicode codepoint;
		Engine::Box2D atlasbounds;
		Engine::Box2D planebounds;
		float advance;
		uint32 index;
	};

	struct PH_ENGINE_API Font : public AssetBase {

		//asset needs this to compile templated function getAsset
		static AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_FONT;
		}

		Engine::Texture atlas;
		Engine::String source;
		Engine::ArrayList<Glyph> glyphs;

		Engine::StaticGPUBuffer<Engine::Box2D> uvbuffer;
		Platform::GFX::DescriptorSet descriptorset;

		static Platform::GFX::DescriptorSetLayout createDescriptorSetLayout();
		static Platform::GFX::DescriptorSetLayout descriptorsetlayout;
	};

	void serializeFont(const char* filepath, Font* font);
	void deserializeFont(const char* filepath, Font* font);
	Font createFont(const char* ttf, float fontsize);

	AssetDescription createFontDescription();

	glm::vec2 drawText(Font* font, const char* text, glm::vec2 position, real32 scale, const glm::vec4& color, Engine::Renderer2D::Context* context);
	
	//returns the length in pixels of the given text;
	real32 getTextLength(Font* font, Base::SubString textstring, real32 scale);
	glm::vec2 getCharPosition(Font* font, Base::SubString textstring, real32 scale, uint32 position);

	inline const char* getFontExtension() {
		return ".lcfont";
	}

}