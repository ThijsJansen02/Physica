#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"


#include <stb/stb_image.h>

namespace PH::LabCore3D {

	using namespace PH::Platform;

	LabCore3D::Texture createTexture(const char* sourcepath) {

		Platform::FileBuffer f;
		Platform::loadFile(&f, sourcepath);

		Texture r;

		int c, creq;
		creq = 4;
		int w, h;
		uint8* im = stbi_load_from_memory((stbi_uc*)f.data, f.size, &w, &h, &c, creq);

		PH::Platform::GFX::TextureCreateInfo ai{};
		ai.data = nullptr;

		if (c == 4) {
			ai.format = GFX::FORMAT_R8G8B8A8_SRGB;
		}

		if (c == 3) {
			ai.format = GFX::FORMAT_R8G8B8_SRGB;
		}

		ai.format = GFX::FORMAT_R8G8B8A8_SRGB;

		ai.data = im;
		ai.height = w;
		ai.width = h;
		ai.usage = GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		GFX::Texture texture;

		if (!GFX::createTextures(&ai, &texture, 1)) {
			Engine::ERR << "Failed to create Albedo render target!\n";
			return {};
		}

		r.format = ai.format;
		r.texture = texture;
		r.width = w;
		r.height = h;

		stbi_image_free(im);
		Platform::unloadFile(&f);

		return r;
	}

	void serializeTexture(const YAML::Emitter& out, Texture* tex) {

	}

	void deserializeTexture(YAML::Node node, Texture* tex) {

	}

	Engine::AssetDescription createTextureAssetDescription() {
		Engine::AssetDescription d;
		d.id = PH_ASSET_TEXTURE;
		d.extension = ".lctex";
		d.localdatasize = sizeof(Texture);
		d.serialize_ = (Engine::SerializeAsset)serializeTexture;
		d.deserialize_ = (Engine::deserializeAsset)deserializeTexture;

		return d;
	}


}