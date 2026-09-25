
#include "LabCore.h"
#include "View.h"

namespace PH::LabCore {


	const ViewInstance* openview = nullptr;

	//updates all views in the application, this is called by the application in the update loop
	void updateViews(const Engine::ArrayList<ViewInstance>& views) {
		for (auto& viewinstance : views) {

			openview = &viewinstance;
			if (openview->view->onUpdate_) {
				openview->view->onUpdate_(viewinstance.instancedata);
			}
		}
		//reset the open view to nullptr
		openview = nullptr;
	}

	//draws all views in the application, this is called by the application in the update loop
	void drawViews(const Engine::ArrayList<ViewInstance>& views) {
		for (auto& viewinstance : views) {

			openview = &viewinstance;
			if (openview->view->draw_) {

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
				if (ImGui::Begin(openview->view->name.getC_Str())) {
					ImVec2 windowpos = ImGui::GetWindowPos();
					ImVec2 windowsize = ImGui::GetWindowSize();

					openview->view->draw_(viewinstance.instancedata);

					ImGui::End();
				}
				ImGui::PopStyleVar(1);
			}
		}

		openview = nullptr;
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