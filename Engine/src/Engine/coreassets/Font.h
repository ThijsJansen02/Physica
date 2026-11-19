#pragma once
#include <Platform/platformAPI.h>
#include <Engine/AssetDescription.h>

#include <Engine/coreassets/Texture.h>
#include <Engine/Rendering.h>

namespace PH::Engine {

//random hardcoded UUID for the font asset description
#define PH_ASSET_FONT 8122762271555803

	
	/*
	struct Glyph {
		int index;
		unicode_t codepoint;
		double geometryScale;
		msdfgen::Shape shape;
		msdfgen::Shape::Bounds bounds;
		double advance;
		struct {
			struct {
				int x, y, w, h;
			} rect;
			double range;
			double scale;
			glm::vec2 translate;
		} box;
	};

	*/

	struct Font : public AssetBase {

		//asset needs this to compile templated function getAsset
		static AssetDescriptionID getAssetDescriptionID() {
			return PH_ASSET_FONT;
		}

		Engine::Texture atlas;

		Engine::String source;

		//Engine::ArrayList<Glyph> glyphs;


		uint32 atlaswidth;
		real32 atlasheight;

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