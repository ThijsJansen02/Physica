

#include "PlotViewPanel.h"
#include "RpGui.h"
#include <Engine/Events.h>


namespace PH::RpGui {
	
	PlotViewPanel PlotViewPanel::create(Box2D range, const char* name)  {

		PlotViewPanel result;
		result.display = Engine::createImGuiDisplay(1920, 1080);
		result.region.left = 0.0f;
		result.region.right = (real32)result.display.framebuffersize.x;
		result.region.bottom = 0.0f;
		result.region.top = (real32)result.display.framebuffersize.y;

		result.name = Engine::String::create(name);

		result.range = range;

		result.isfocussed = false;

		return result;

	}

	void PlotViewPanel::onEvent(PH::Platform::Event* event) {

		real32 scrollspeed = 0.001f;

		if (event->type == PH_EVENT_TYPE_MOUSE_SCROLLED && ishovered) {
			int16 delta = (int16)(void*)event->lparam;

			real32 zoom = 1.0f - (real32)delta * scrollspeed;

			glm::vec2 mousepos = Engine::Events::getMousePos();
			mousepos.y = Engine::getParentDisplay()->viewport.y - mousepos.y; //flip y coordinate because the window coordinate system has y going down and the plot coordinate system has y going up

			mousepos.x -= panelregion.left; //subtract the left coordinate of the panel region from the x coordinate of the mouse position, because the panel region is not necessarily at the left edge of the window and we want to use the panel region for transforming mouse coordinates from window coordinates to plot coordinates
			mousepos.y -= panelregion.bottom; //subtract the bottom coordinate of the panel region from the y coordinate of the mouse position, because the panel region is not necessarily at the bottom edge of the window and we want to use the panel region for transforming mouse coordinates from window coordinates to plot coordinates

			//transform mousepos from window coordinates to plot coordinates
			mousepos.x = range.left + (mousepos.x / (real32)region.right) * (range.right - range.left);
			mousepos.y = range.bottom + (mousepos.y / (real32)region.top) * (range.top - range.bottom);
			//Engine::INFO << "mouse pos: " << mousepos.x << ", " << mousepos.y << "\n";	


			//update mouseevents
			if (PH::Engine::Events::isKeyPressed(PH_CONTROL)) {
				range.right = (range.right - mousepos.x) * zoom + mousepos.x;
				range.left = (range.left - mousepos.x) * zoom + mousepos.x;
			}
			else {
				range.top = (range.top - mousepos.y) * zoom + mousepos.y;
				range.bottom = (range.bottom - mousepos.y) * zoom + mousepos.y;
			}

			if (xlock) {
				xlock->range.left = range.left;
				xlock->range.right = range.right;
			}
		}
	}

	void PlotViewPanel::ImGuiDraw() {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		auto& io = ImGui::GetIO();

		if (ImGui::Begin(name.getC_Str())) {
			real32 titlebarheight = ImGui::GetFrameHeight();

			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, 0 });

			ImVec2 displaysize = ImGui::GetContentRegionAvail();
			display.viewport = { displaysize.x, displaysize.y };


			ImVec2 windowpos = ImGui::GetWindowPos();
			panelregion.left = windowpos.x;
			panelregion.top = Engine::getParentDisplay()->viewport.y - windowpos.y; //file the y coordinate because the window coordinate system has y going down and the plot coordinate system has y going up
			panelregion.top -= titlebarheight; //subtract the titlebar height from the top coordinate of the panel region, because the titlebar is not part of the content region and we want to use the content region for drawing the plot

			panelregion.right = windowpos.x + displaysize.x;
			panelregion.bottom = Engine::getParentDisplay()->viewport.y - (windowpos.y + displaysize.y); //flip all y coordinates because the window coordinate system has y going down and the plot coordinate system has y going upGui::GetWindowPos().y + ImGui::GetContentRegionAvail().y;

			//update region to the size of the display;
			region.left = 0.0f;
			region.right = abs(display.viewport.x);
			region.bottom = 0.0f;
			region.top = abs(display.viewport.y);

			ImGui::Image(
				display.imguitexture,
				displaysize,
				{ 0.0f, 0.0f }, //bottom left uv
				{ displaysize.x / display.framebuffersize.x, displaysize.y / display.framebuffersize.y } //top right UV
			);

			//might move this to a update function in the future , for now its just easier to do it here
			if(ImGui::IsItemHovered()) {
				ishovered = true;
				glm::vec2 mousefactor = { (range.right - range.left) / (region.right - region.left), (range.top - range.bottom) / (region.top - region.bottom) };
				if (PH::Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {

					glm::vec2 deltam = PH::Engine::Events::getDeltaMousePos();

					range.left -= mousefactor.x * deltam.x;
					range.right -= mousefactor.x * deltam.x;
					range.top += mousefactor.y * deltam.y;
					range.bottom += mousefactor.y * deltam.y;

					if (xlock) {
						xlock->range.left = range.left;
						xlock->range.right = range.right;
					}
				}
			}
			else {
				ishovered = false;
			}
		}

		ImGui::End();

		ImGui::PopStyleVar(1);
	}

	void PlotViewPanel::serialize(YAML::Emitter& out) {

		out << YAML::Key << this->name.getC_Str() << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "range" << YAML::Value << YAML::BeginSeq << range.left << range.bottom << range.right << range.top << YAML::EndSeq;
		out << YAML::EndMap;
	}

	void PlotViewPanel::deserialize(const YAML::Node& root) {

		const auto& range_ = root["range"];
		range.left = range_[0].as<real32>();
		range.bottom = range_[1].as<real32>();
		range.right = range_[2].as<real32>();
		range.top = range_[3].as<real32>();
		
	}

	void PlotViewPanel::draw() {

		
	}



}