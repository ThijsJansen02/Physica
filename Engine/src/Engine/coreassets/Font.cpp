#include "Font.h"


#undef INFINITE
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include <Engine/coreassets/Texture.h>


#include <Engine/Rendering.h>

namespace PH::Engine {

	using namespace msdf_atlas;

	Platform::GFX::DescriptorSetLayout Font::descriptorsetlayout = PH_GFX_NULL;

	Font loadMSDFatlas(const char* source) {

		Font result{};

		// Initialize instance of FreeType library
		if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
			// Load font file using in house file loader

			PH::Platform::FileBuffer f;
			if (!PH::Platform::loadFile(&f, source)) {
				return {};
			}

			if (msdfgen::FontHandle* font = msdfgen::loadFontData(ft, (msdfgen::byte*)f.data, f.size)) {
				// Storage for glyph geometry and their coordinates in the atlas
				std::vector<GlyphGeometry> glyphs;
				// FontGeometry is a helper class that loads a set of glyphs from a single font.
				// It can also be used to get additional font metrics, kerning information, etc.
				FontGeometry fontGeometry(&glyphs);
				// Load a set of character glyphs:
				// The second argument can be ignored unless you mix different font sizes in one atlas.
				// In the last argument, you can specify a charset other than ASCII.
				// To load specific glyph indices, use loadGlyphs instead.
				fontGeometry.loadCharset(font, 1.0, Charset::ASCII);
				// Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
				const double maxCornerAngle = 3.0;
				for (GlyphGeometry& glyph : glyphs)
					glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
				// TightAtlasPacker class computes the layout of the atlas.
				TightAtlasPacker packer;
				// Set atlas parameters:
				// setDimensions or setDimensionsConstraint to find the best value
				packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::SQUARE);
				// setScale for a fixed size or setMinimumScale to use the largest that fits
				packer.setMinimumScale(24.0);
				// setPixelRange or setUnitRange
				packer.setPixelRange(2.0);
				packer.setMiterLimit(1.0);
				// Compute atlas layout - pack glyphs
				packer.pack(glyphs.data(), glyphs.size());
				// Get final atlas dimensions
				int width = 0, height = 0;
				packer.getDimensions(width, height);
				// The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
				ImmediateAtlasGenerator<
					float, // pixel type of buffer for individual glyphs depends on generator function
					4, // number of atlas color channels
					mtsdfGenerator, // function to generate bitmaps for individual glyphs
					BitmapAtlasStorage<byte, 4> // class that stores the atlas bitmap
					// For example, a custom atlas storage class that stores it in VRAM can be used.
				> generator(width, height);
				// GeneratorAttributes can be modified to change the generator's default settings.
				GeneratorAttributes attributes;
				generator.setAttributes(attributes);
				generator.setThreadCount(4);
				// Generate atlas bitmap
				generator.generate(glyphs.data(), glyphs.size());
				// The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
				// The glyphs array (or fontGeometry) contains positioning data for typesetting text.

				msdfgen::BitmapConstRef<byte, 4> bitmap = generator.atlasStorage();


				result.atlas = Engine::createTexture(Base::Array<uint8>{(uint8*)bitmap.pixels, (sizeptr)bitmap.width * (sizeptr)bitmap.height }, (uint32)bitmap.width, (uint32)bitmap.height, Platform::GFX::FORMAT_R8G8B8A8_UNORM);
				//result.glyphs = Engine::ArrayList<Engine::Glyph>::create({ (Engine::Glyph*)&glyphs[0], glyphs.size() });

				// Cleanup
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}
		return result;
	}

	Font createFont(const char* ttf, float fontsize) {

		Font r = loadMSDFatlas(ttf);

		return r;
	}

	glm::vec2 drawText(Font* font, const char* text, glm::vec2 position, real32 scale, const glm::vec4& color, Engine::Renderer2D::Context* context) {
		return { 0, 0 };

	}

	//returns the length in pixels of the given text;
	real32 getTextLength(Font* font, Base::SubString textstring, real32 scale) {
		return 0;
	}
	glm::vec2 getCharPosition(Font* font, Base::SubString textstring, real32 scale, uint32 position) {
		return { 0, 0 };
	}


	Platform::GFX::DescriptorSetLayout createFontDescriptorSetLayout() {

		Platform::GFX::DescriptorBinding binding;
		binding.binding = 0;
		binding.descriptorcount = 1;
		binding.shaderstages = Platform::GFX::SHADER_STAGE_FRAGMENT_BIT;
		binding.type = Platform::GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Platform::GFX::DescriptorSetLayoutCreateinfo createinfo{};
		createinfo.bindings = { &binding, 1 };
		Platform::GFX::DescriptorSetLayout result;
		Platform::GFX::createDescriptorSetLayouts(&createinfo, &result, 1);
		return result;
	}

	AssetDescription createFontDescription() {
		AssetDescription description;
		description.deserialize_ = (deserializeAsset)deserializeFont;
		description.serialize_ = (SerializeAsset)serializeFont;
		description.localdatasize = sizeof(Font);
		description.id = PH_ASSET_FONT;
		description.extension = getFontExtension();

		return description;
	}

	void serializeFont(const char* filepath, Font* font) {

	}

	void deserializeFont(const char* filepath, Font* font) {

	}

	struct UVcoords {
		glm::vec2 bottomleft;
		glm::vec2 topright;
	};

}