
#include "LabCore.h"
#include "View.h"
#include <Engine/Events.h>

namespace PH::LabCore {


	ViewInstance* openview = nullptr;

	//updates all views in the application, this is called by the application in the update loop
	void updateViews(Engine::ArrayList<ViewInstance*>& views) {
		for (auto& viewinstance : views) {

			openview = viewinstance;
			if (openview->viewdescription->onUpdate_) {
				openview->viewdescription->onUpdate_(viewinstance->instancedata);
			}
		}
		//reset the open view to nullptr
		openview = nullptr;
	}

	Engine::Box2D getImGuiWindowContentRegion() {

		ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
		ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
		ImVec2 windowPos = ImGui::GetWindowPos();

		auto* maindisplay = Engine::getParentDisplay();
		
		glm::vec2 windowpos_lb = { windowPos.x, maindisplay->viewport.y - (windowPos.y + contentMax.y) };
		real32 crwidth = contentMax.x - contentMin.x;
		real32 crheight = contentMax.y - contentMin.y;

		return Engine::Box2D::create(windowpos_lb.x, windowpos_lb.x + crwidth, windowpos_lb.y + crheight, windowpos_lb.y);
	}

	//draws all views in the application, this is called by the application in the update loop
	void drawViews(Engine::ArrayList<ViewInstance*>& views) {

		ViewInstance* tobedeleted = nullptr;
		for (auto* viewinstance : views) {

			openview = viewinstance;
			if (openview->viewdescription->draw_) {

				real32 framesize = ImGui::GetWindowSize().y;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, openview->viewdescription->windowpadding);

				char windownamebuffer[256];
				sprintf_s(windownamebuffer, 256, "%s###%llu", openview->name.getC_Str(), openview->instanceid);

				bool isopen = true;
				if (ImGui::Begin(windownamebuffer, &isopen, openview->viewdescription->windowflags)) {
					
					openview->region = getImGuiWindowContentRegion();
					openview->isfocussed = ImGui::IsWindowFocused();
					openview->ishovered = ImGui::IsWindowHovered();
					openview->viewdescription->draw_(viewinstance->instancedata);


					glm::vec2 mousepos = Engine::Events::getMousePos();

				}

				if (isopen == false) {
					tobedeleted = viewinstance;
				}

				ImGui::End();
				ImGui::PopStyleVar(1);
			}

		}
		if (tobedeleted) {
			destroyViewInstance(tobedeleted);
			Engine::Allocator::dealloc(tobedeleted);
			views.removeAllInstances(tobedeleted);
		}
		openview = nullptr;
	}

	ViewInstance createViewInstance(ViewDescription* view) {
		return createViewInstance(view, Engine::createRandomUUID());
	}

	ViewInstance createViewInstance(ViewDescription* description, Engine::UUID id) {
		ViewInstance instance{};
		instance.viewdescription = description;
		instance.instancedata = PH::Engine::Allocator::alloc(description->instancedatasize);
		instance.name = Engine::String::create(description->name.getC_Str());
		instance.instanceid = id;

		description->instantiate_(instance.instancedata);
		return instance;
	}

	void destroyViewInstance(ViewInstance* view) {
		if (view->viewdescription->destroy_ && view->instancedata) {
			view->viewdescription->destroy_(view->instancedata);
		}

		if (view->instancedata) {
			Engine::Allocator::dealloc(view->instancedata);
		}

		Engine::String::destroy(&view->name);
	}

	
	bool32 isCursorOnView(ViewInstance* view) {
		glm::vec2 mousepos = Engine::Events::getMousePos();
		return Engine::isInBox2D(view->region, mousepos);
	}

	//passes the event trough to the focussed view
	bool32 interpretEventForViews(Engine::ArrayList<ViewInstance*>& views, const PH::Platform::Event& e) {
		for (auto* viewinstance : views) {

			openview = viewinstance;

			if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
				if (isCursorOnView(viewinstance)) {
					//viewinstance.isfocussed = true;
					if (viewinstance->viewdescription->onEvent_) {
						return viewinstance->viewdescription->onEvent_(viewinstance->instancedata, e);
					}

				}
				viewinstance->isfocussed = false;
			}

			if (viewinstance->isfocussed) {
				if (viewinstance->viewdescription->onEvent_) {
					return viewinstance->viewdescription->onEvent_(viewinstance->instancedata, e);
				}
			}
		}
		openview = nullptr;
	}

	void destroyAllViewInstances(Engine::ArrayList<ViewInstance*>* views) {
		for (auto* view : *views) {
			destroyViewInstance(view);
			Engine::Allocator::dealloc(view);
		}
		views->clear();
	}
}