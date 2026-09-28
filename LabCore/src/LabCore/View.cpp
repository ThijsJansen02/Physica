
#include "LabCore.h"
#include "View.h"
#include <Engine/Events.h>

namespace PH::LabCore {


	ViewInstance* openview = nullptr;

	//updates all views in the application, this is called by the application in the update loop
	void updateViews(Engine::ArrayList<ViewInstance>& views) {
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
	void drawViews(Engine::ArrayList<ViewInstance>& views) {
		for (auto& viewinstance : views) {

			openview = &viewinstance;
			if (openview->view->draw_) {

				real32 framesize = ImGui::GetWindowSize().y;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
				if (ImGui::Begin(openview->view->name.getC_Str(), nullptr, ImGuiWindowFlags_MenuBar)) {
					
					openview->isfocussed = ImGui::IsWindowFocused();

					//determine the window region
					ImVec2 windowpos = ImGui::GetWindowPos();
					ImVec2 windowsize = ImGui::GetWindowSize();

					windowpos.y = framesize - windowpos.y;

					Engine::Box2D windowregion;
					windowregion.bottomleft = { windowpos.x, windowpos.y - windowsize.y };
					windowregion.topright = { windowpos.x + windowsize.x, windowpos.y };
					
					openview->region = windowregion;

					glm::vec2 mousepos = Engine::Events::getMousePos();

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
	
	bool32 isCursorOnView(const ViewInstance& view) {
		glm::vec2 mousepos = Engine::Events::getMousePos();
		return Engine::isInBox2D(view.region, mousepos);
	}

	//passes the event trough to the focussed view
	bool32 interpretEventForViews(Engine::ArrayList<ViewInstance>& views, const PH::Platform::Event& e) {
		for (auto& viewinstance : views) {

			if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
				if (isCursorOnView(viewinstance)) {
					viewinstance.isfocussed = true;
					return viewinstance.view->onEvent_(viewinstance.instancedata, e);
				}
				viewinstance.isfocussed = false;
			}

			if (viewinstance.isfocussed) {
				return viewinstance.view->onEvent_(viewinstance.instancedata, e);
			}
		}
	}

}