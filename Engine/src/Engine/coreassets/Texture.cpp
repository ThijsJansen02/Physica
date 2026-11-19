#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"


#include <stb/stb_image.h>

namespace PH::Engine {

	using namespace PH::Platform;

	uint32 getFormatComponentCount(GFX::Format format) {
		switch (format) {
		case GFX::FORMAT_R8G8B8A8_UNORM:
		case GFX::FORMAT_R8G8B8A8_SRGB:
			return 4;
		case GFX::FORMAT_R8G8B8_UNORM:
		case GFX::FORMAT_R8G8B8_SRGB:
			return 3;
		case GFX::FORMAT_R8G8_SRGB:
		case GFX::FORMAT_R8G8_UNORM:
			return 2;
		case GFX::FORMAT_R8_UNORM:
		case GFX::FORMAT_R8_SRGB:
		case GFX::FORMAT_R32_UINT:
			return 1;
		default:
			return 0;
		}
	}

	Engine::Texture createTexture(const Platform::FileBuffer& f, GFX::Format format, GFX::ImageUsageFlags imageusage) {

		Texture r;
		r.height = 0;
		r.width = 0;
		r.texture = PH_GFX_NULL;

		int c, creq;
		creq = getFormatComponentCount(format);
		int w, h;

		uint8* im = stbi_load_from_memory((stbi_uc*)f.data, f.size, &w, &h, &c, creq);

		if (!im) {
			Engine::WARN << "failed to load texture";
			return r;
		}

		PH::Platform::GFX::TextureCreateInfo ai{};
		ai.data = nullptr;

		ai.format = format;

		ai.data = im;
		ai.height = h;
		ai.width = w;
		ai.usage = imageusage;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create image!\n";
			return {};
		}

		r.format = ai.format;
		r.texture = texture;
		r.width = w;
		r.height = h;

		stbi_image_free(im);

		return r;
	}

	Engine::Texture createTexture(const char* sourcepath, GFX::Format format) {
		Platform::FileBuffer f;
		Platform::loadFile(&f, sourcepath);

		Texture r = createTexture(f, format, GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_TRANSFER_DST_BIT);

		Platform::unloadFile(&f);

		return r;
	}


	Engine::Texture createTexture(const char* sourcepath) {

		return createTexture(sourcepath, GFX::FORMAT_R8G8B8A8_SRGB);
	}

	Texture createTexture(Base::Array<uint8> bitmap, uint32 width, uint32 height, PH::Platform::GFX::Format format) {

		Texture r{};
		r.format = format;
		r.width = width;
		r.source = Engine::String::create("");
		r.height = height;

		PH::Platform::GFX::TextureCreateInfo inf;
		inf.ArrayLayers = 1;
		inf.data = bitmap.data;
		inf.width = width;
		inf.height = height;
		inf.usage = GFX::IMAGE_USAGE_SAMPLED_BIT;
		inf.viewtype = GFX::IMAGE_VIEW_TYPE_2D;
		inf.format = format;

		if (!GFX::createTextures(&inf, &r.texture, 1)) {
			Engine::WARN << "Failed to load texure from bitmap!";
			return {};
		}

		return r;	
	}

	void serializeTexture(YAML::Emitter& out, Texture* tex) {
		
		out << YAML::BeginMap;
		out << YAML::Key << "width" << YAML::Value << tex->width;
		out << YAML::Key << "height" << YAML::Value << tex->height;
		out << YAML::Key << "format" << YAML::Value << (uint32)tex->format;
		out << YAML::Key << "source" << YAML::Value << tex->source.getC_Str();
		out << YAML::EndMap;
	}

	void deserializeTexture(YAML::Node node, Texture* tex) {
		
		GFX::Format format = (GFX::Format)node["format"].as<uint32>();
		tex->width = node["width"].as<sizeptr>();
		tex->height = node["height"].as<sizeptr>();
		Engine::String source = node["source"].as<Engine::String>();
		*tex = createTexture(source.getC_Str(), format);
		tex->source = source;
	}

	Engine::AssetDescription createTextureAssetDescription() {
		Engine::AssetDescription d{};
		d.destroy_ = nullptr;
		d.id = PH_ASSET_TEXTURE;
		d.extension = ".lctex";
		d.localdatasize = sizeof(Texture);
		d.serialize_ = (Engine::SerializeAsset)serializeTexture;
		d.deserialize_ = (Engine::deserializeAsset)deserializeTexture;

		return d;
	}


}