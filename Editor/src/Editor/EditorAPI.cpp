#pragma once
#include "EditorAPI.h"

namespace PH::Editor::API {

	Editor::Instance* editor_s;

	Instance* getCurrentInstance() {
		return editor_s;
	}

	void setCurrentInstance(Instance* instance) {
		editor_s = instance;
	}


}