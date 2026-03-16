#pragma once
#include <Engine/AssetLibrary.h>
#include <Engine/Engine.h>
#include <Engine/assets/Material.h>
#include "../EditorAPI.h"

namespace PH::Editor {

	struct Instance;

	struct MaterialView {
		

		Engine::Assets::Material* lastselectedmaterial;
		Engine::Assets::Material* selectedmaterial;

		Engine::Assets::MaterialInstance* instance;

		bool32 focussed;
	};

	void materialViewOnEvent(PH::Platform::Event e, Editor::Instance* instance, MaterialView* view);

	MaterialView createMaterialView();
	void drawMaterialView(Editor::Instance* instance, MaterialView* view);

#define MATERIALVIEW_ID 4
	inline bool32 materialView_draw(void* userdata) {
		MaterialView* view = (MaterialView*)userdata;
		Editor::Instance* editor = API::getCurrentInstance();

		drawMaterialView(editor, view);

		return true;
	}

	inline bool32 materialView_dispatchEvent(void* userdata, const PH::Platform::Event& e) {

		MaterialView* view = (MaterialView*)userdata;
		Editor::Instance* editor = API::getCurrentInstance();

		materialViewOnEvent(e, editor, view);

		return true;
	}

	inline View loadMaterialView() {

		View view{};
		view.dispatchEvent = materialView_dispatchEvent;
		view.draw = materialView_draw;
		view.userdata = Engine::Allocator::alloc(sizeof(MaterialView));
		*(MaterialView*)view.userdata = createMaterialView();
		view.name = Engine::String::create("MaterialView");
		view.id = Library::createRandomUUID();
		view.sourceid = MATERIALVIEW_ID;
		return view;

	}

}