#include "TextureImport.h"
#include <Engine/Engine.h>
#include <Engine/AssetLibrary.h>
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Texture.h>


namespace PH::LabCore3D {

	using namespace PH::Platform;

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

				Engine::Texture* tex = Engine::Allocator::push(Engine::createTexture(f, format, GFX::IMAGE_USAGE_SAMPLED_BIT | GFX::IMAGE_USAGE_TRANSFER_DST_BIT));
				tex->source = Engine::String::create(importingTexturePath.getC_Str());

				if (tex->texture != PH_GFX_NULL) {
					LabCore::getAssetLibrary()->addAsset<Engine::Texture>(tex, internalfilepath.getC_Str(), filename.getC_Str(), Engine::PH_ASSET_SERIALIZABLE);
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

}