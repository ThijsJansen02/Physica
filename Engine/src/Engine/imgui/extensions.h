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

	inline PH::bool32 Input(const char* label, Engine::String* text) {
		//
		char buffer[256];
		Base::stringCopy(text->getC_Str(), buffer, 256);
		if(ImGui::InputText(label, buffer, 256)) {
			text->set(buffer);
			return true;
		}

		return false;
	}
}