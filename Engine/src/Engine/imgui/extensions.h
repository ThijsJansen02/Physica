#pragma once
#include <imgui.h>
#include <Engine/Engine.h>
#include <Engine/AssetLibrary.h>

namespace PH::ImGuiext {

	inline PH::Engine::AssetIdentifier* assetSelecter(const char* label, PH::Engine::AssetIdentifier* selected, Base::Array<Engine::AssetIdentifier*> options, const char* assetstr) {

		Engine::AssetIdentifier* newselected = selected;

		if (ImGui::BeginCombo(label, selected ? selected->getName() : assetstr)) {

			for (auto* asset : options) {
				if (ImGui::Selectable(asset->getName(), asset == selected)) {
					newselected = asset;
				}
			}

			ImGui::EndCombo();
		}
		return newselected;
	}

	template<typename Asset>
	inline Asset* assetSelecter(const char* label, Asset* selected, const char* assetstr, Engine::AssetLibrary* lib) {
		
		Engine::ArrayList<Engine::AssetIdentifier*> options = lib->getAllIdentifiersOfType<Asset>();

		Asset* newselected = selected;

		if (ImGui::BeginCombo(label, selected ? selected->identifier->getName() : assetstr)) {
			for (Engine::AssetIdentifier* identifier : options) {
				if (ImGui::Selectable(identifier->getName(), selected == (Asset*)identifier->assetdata)) {
					if (identifier->status != Engine::LOADED) {
						lib->loadAsset<Asset>(identifier->assetid);
					}
					newselected = (Asset*)identifier->assetdata;
				}
			}
			ImGui::EndCombo();
		}
		Engine::ArrayList<Engine::AssetIdentifier*>::destroy(&options);
		return newselected;
	}

	template<typename Asset>
	PH::Engine::AssetIdentifier* assetSelecter(const char* label, PH::Engine::AssetIdentifier* selected, const char* assetstr, Engine::AssetLibrary* lib) {

		Engine::ArrayList<Engine::AssetIdentifier*> options = lib->getAllIdentifiersOfType<Asset>();

		Engine::AssetIdentifier* newselected = selected;

		if (ImGui::BeginCombo(label, selected ? selected->getName() : assetstr)) {

			for (auto* asset : options) {
				if (ImGui::Selectable(asset->getName(), asset == selected)) {
					newselected = asset;
					if (newselected->status != Engine::LOADED) {
						lib->loadAsset<Asset>(newselected->assetid);
					}

				}
			}

			ImGui::EndCombo();
		}

		Engine::ArrayList<Engine::AssetIdentifier*>::destroy(&options);
		return newselected;
	}

	inline PH::bool32 Input(const char* label, Engine::String* text, ImGuiInputTextFlags flags) {
		char buffer[256];
		Base::stringCopy(text->getC_Str(), buffer, 256);
		if (ImGui::InputText(label, buffer, 256, flags)) {
			text->set(buffer);
			return true;
		}
		return false;
	}

	inline PH::bool32 Input(const char* label, Engine::String* text) {
		return Input(label, text, 0);
	}

	inline PH::bool32 FilePath(const char* label, Engine::String* path, const char* filter, const char* startingdir, uint32 dialogflags) {
		bool r = false;
		
		uint64 strhash = Base::stringHash(label);

		ImGui::PushID((int)strhash);
		if (ImGui::Button("...")) {
			char buffer[256];
			PH::Platform::OpenFileDialogInfo info{};
			info.filter = filter;
			info.flags = dialogflags;
			info.resultbuffer = buffer;
			info.resultbuffersize = 256;
			info.read = false;
			info.startingdir = startingdir;
			if (PH::Platform::openFileDialog(info)) {
				path->set(buffer);
				r |= true;
			}
		}
		ImGui::SameLine();
		r |= ImGuiext::Input(label, path, 0);
		ImGui::PopID();
		
		return r;
	}
}