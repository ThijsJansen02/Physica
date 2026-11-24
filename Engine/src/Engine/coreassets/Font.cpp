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
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8
		// if MSDF || MTSDF

				uint64_t coloringSeed = 20;
				bool expensiveColoring = true;
				if (expensiveColoring)
				{
					msdf_atlas::Workload([&glyphs = glyphs, &coloringSeed](int i, int threadNo) -> bool {
						unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
						glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
						return true;
						}, glyphs.size()).finish(THREAD_COUNT);
				}
				else {
					unsigned long long glyphSeed = coloringSeed;
					for (msdf_atlas::GlyphGeometry& glyph : glyphs)
					{
						glyphSeed *= LCG_MULTIPLIER;
						glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
					}
				}

				float emsize = fontGeometry.getMetrics().emSize;
				msdf_atlas::TightAtlasPacker packer;
				// Set atlas parameters:
				// setDimensions or setDimensionsConstraint to find the best value
				packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::SQUARE);
				// setScale for a fixed size or setMinimumScale to use the largest that fits
				packer.setMinimumScale(40.0);
				// setPixelRange or setUnitRange
				packer.setPixelRange(8.0);
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
				result.glyphs = Engine::ArrayList<Engine::Glyph>::create(glyphs.size());

				uint32 index = 0;
				for (auto g : glyphs) {
					Engine::Glyph fg;
					fg.advance = g.getAdvance();

					fg.codepoint = g.getCodepoint();

					real64 l, r, t, b;
					g.getQuadAtlasBounds(l, b, r, t);
					fg.atlasbounds = Box2D::create(l / (real32)width, r / (real32)width, t / (real32)height, b / (real32)height);

					g.getQuadPlaneBounds(l, b, r, t);
					fg.planebounds = Box2D::create(l, r, t, b);


					fg.index = index++;

					result.glyphs.pushBack(fg);
				}

				// Cleanup
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}
		return result;
	}

	using namespace PH::Platform;

	static StaticGPUBuffer<Engine::Box2D> createFontUVbuffer(Base::Array<Glyph> glyphs) {

		Engine::ArrayList<Box2D> uvs = Engine::ArrayList<Box2D>::create(glyphs.count);

		for (const auto& g : glyphs) {

			uvs.pushBack(g.atlasbounds);

		}

		return StaticGPUBuffer<Engine::Box2D>::create(uvs.getArray(), GFX::BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}

	static GFX::DescriptorSet createFontDescriptorSet(const StaticGPUBuffer<Engine::Box2D>& uvbuffer) {
		GFX::DescriptorSet r{};

		GFX::DescriptorSetCreateinfo ci{};
		ci.dynamic = false;
		ci.layout = Font::descriptorsetlayout;
		if (!GFX::createDescriptorSets(&ci, &r, 1)) {
			return PH_GFX_NULL;
		}

		GFX::DescriptorBufferInfo bi{};
		bi.buffer = uvbuffer.buffer;
		bi.offset = 0;
		bi.range = uvbuffer.capacity * sizeof(Engine::Box2D);

		GFX::DescriptorWrite wr{};
		wr.arrayelement = 0;
		wr.descriptorcount = 1;
		wr.descriptorinfo = &bi;
		wr.dstbinding = 0;
		wr.dstset = r;
		wr.dynamicwrite = false;
		wr.type = GFX::DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		GFX::updateDescriptorSets(&wr, 1);
		return r;
	}

	Font createFont(const char* ttf, float fontsize) {

		Font r = loadMSDFatlas(ttf);
		r.uvbuffer = createFontUVbuffer(r.glyphs.getArray());
		r.descriptorset = createFontDescriptorSet(r.uvbuffer);
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


	Platform::GFX::DescriptorSetLayout Font::createDescriptorSetLayout() {

		Platform::GFX::DescriptorBinding binding{};
		binding.updateafterbind = false;
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