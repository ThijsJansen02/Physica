#pragma once
#include "../Editor.h"

namespace PH::Editor {

	template<typename type>
	inline bool32 selectAssetCombo(type** selectedptr, const char* name, Editor::Instance* editor, uint64 id, bool lazyload) {
		
		Engine::ArenaScope s;
		
		//should cache these options in the future
		auto options = editor->assets->getAll<type, Engine::ArenaAllocator>();
		type* selected = *selectedptr;

		ImGui::PushID(id);

		bool32 returnvalue = false;
		if (ImGui::BeginCombo(name, (selected != nullptr) ? selected->references[0].getC_Str() : "None")) {

			bool isSelected = selected == nullptr;
			if (ImGui::Selectable("None", isSelected)) {
				*selectedptr = nullptr;
				returnvalue = true;
			}

			for (auto option : options) {
				isSelected = selected == option;

				if (ImGui::Selectable(option->references[0].getC_Str(), isSelected)) {
					if (lazyload) {
						*selectedptr = editor->assets->getLazyLoaded<type>(option->id);
					} else {
						*selectedptr = editor->assets->getLoaded<type>(option->id);
					}
					returnvalue = true;
				}
				 
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::PopID();
		options.release();
		return returnvalue;
	}

	template<typename type>
	inline bool32 selectAssetCombo(type** selectedptr, const char* name, Editor::Instance* editor) {
		return selectAssetCombo<type>(selectedptr, name, editor, 0, false);
	}
}