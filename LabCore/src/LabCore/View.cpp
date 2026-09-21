
#include "LabCore.h"
#include "View.h"

namespace PH::LabCore {

	//updates all views in the application, this is called by the application in the update loop
	void updateViews(const Engine::ArrayList<ViewInstance>& views) {
		for (auto& viewinstance : views) {

			View* view = viewinstance.view;

			if (view->onUpdate_) {
				view->onUpdate_(viewinstance.instancedata);
			}
		}
	}

	//draws all views in the application, this is called by the application in the update loop
	void drawViews(const Engine::ArrayList<ViewInstance>& views) {
		for (auto& viewinstance : views) {

			View* view = viewinstance.view;
			if (view->draw_) {
				if (ImGui::Begin(view->name.getC_Str())) {
					view->draw_(viewinstance.instancedata);

					ImGui::End();
				}
			}
		}
	}

	ViewInstance createViewInstance(View* view, Engine::UUID id) {
		ViewInstance instance{};
		instance.view = view;
		instance.instancedata = PH::Engine::Allocator::alloc(view->instancedatasize);

		instance.instanceid = id;

		view->instantiate_(instance.instancedata);
		return instance;
	}

	ViewInstance createViewInstance(View* view) {
		return createViewInstance(view, Engine::createRandomUUID());
	}

}