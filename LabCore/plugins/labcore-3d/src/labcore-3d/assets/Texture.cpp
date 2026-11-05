#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"


#include <stb/stb_image.h>

namespace PH::LabCore3D {

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

	LabCore3D::Texture createTexture(const Platform::FileBuffer& f, GFX::Format format, GFX::ImageUsageFlags imageusage) {

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

	LabCore3D::Texture createTexture(const char* sourcepath, GFX::Format format) {
		Platform::FileBuffer f;
		Platform::loadFile(&f, sourcepath);

		Texture r = createTexture(f, format, GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_TRANSFER_DST_BIT);

		Platform::unloadFile(&f);

		return r;
	}

	Engine::String importingTexturePath = {};
	bool32 openTextureImportOptions = false;

	PH::bool32 importTextureCallback(Engine::AssetLibrary* asset, const char* sourcepath) {
		Platform::setCurrentDirectory(PH::LabCore::getCurrentProjectDir());
		
		//create string if it does not exist yet
		if (importingTexturePath.getC_Str() == 0) {
			importingTexturePath = Engine::String::create(sourcepath);
		}
		else {
			importingTexturePath.set(sourcepath);
		}

		openTextureImportOptions = true;
		return true;
	}

	PH::bool32 onRenderTextureImportOptions(Engine::AssetLibrary* assets) {
		
		if (openTextureImportOptions) {
			ImGui::OpenPopup("Texture Import Options");
			openTextureImportOptions = false;
		}


		Platform::setCurrentDirectory(PH::LabCore::getCurrentProjectDir());
		ImGuiIO& io = ImGui::GetIO();

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(
			ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
			ImGuiCond_Appearing,
			ImVec2(0.5f, 0.5f)
		);

		if (ImGui::BeginPopupModal("Texture Import Options", nullptr,
			ImGuiWindowFlags_NoResize))
		{
			PH::Platform::FileBuffer f;

			ImGui::Text("Configure texture import");

			ImGui::Separator();

			static bool generateMipmaps = true;
			static bool sRGB = true;
			static int ncomponents = 4;

			ImGui::Checkbox("Generate mipmaps", &generateMipmaps);
			ImGui::Checkbox("sRGB", &sRGB);

			ImGui::Text("Texture components:");
			ImGui::RadioButton("RGBA", &ncomponents, 4);
			ImGui::RadioButton("RGB", &ncomponents, 3);
			ImGui::RadioButton("RG", &ncomponents, 2);
			ImGui::RadioButton("R", &ncomponents, 1);

			ImGui::Separator();

			if (ImGui::Button("Import")) {
				if (!PH::Platform::loadFile(&f, importingTexturePath.getC_Str())) {
					ImGui::CloseCurrentPopup();
					return false;
				}

				GFX::Format format = GFX::FORMAT_R8G8B8A8_UNORM;
				if (sRGB) {
					switch (ncomponents) {
					case 4: format = GFX::FORMAT_R8G8B8A8_SRGB;
						break;
					case 3: format = GFX::FORMAT_R8G8B8A8_SRGB;
						break;
					case 2: format = GFX::FORMAT_R8G8_SRGB;
						break;
					case 1: format = GFX::FORMAT_R8_SRGB;
						break;
					}
				}
				else {
					switch (ncomponents) {
					case 4: format = GFX::FORMAT_R8G8B8A8_UNORM;
						break;
					case 3: format = GFX::FORMAT_R8G8B8A8_UNORM;
						break;
					case 2: format = GFX::FORMAT_R8G8_UNORM;
						break;
					case 1: format = GFX::FORMAT_R8_UNORM;
						break;
					}
				}

				//make sure the directory exists
				Platform::createDirectory("assets/textures/");

				Engine::String filename = Engine::String::create(Base::getFileName(Base::SubString(importingTexturePath.getC_Str())));
				Engine::String internalfilepath = Engine::String::create("assets/textures/").append(filename).append(".lctex");

				Texture* tex = Engine::Allocator::push(createTexture(f, format, GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_TRANSFER_DST_BIT));
				tex->source = Engine::String::create(importingTexturePath.getC_Str());

				if (tex->texture != PH_GFX_NULL) {
					LabCore::getAssetLibrary()->addAsset<LabCore3D::Texture>(tex, internalfilepath.getC_Str(), filename.getC_Str(), Engine::PH_ASSET_SERIALIZABLE);
				}

				Engine::String::destroy(&filename);
				Engine::String::destroy(&internalfilepath);

 				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}


	LabCore3D::Texture createTexture(const char* sourcepath) {

		return createTexture(sourcepath, GFX::FORMAT_R8G8B8A8_SRGB);
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