

#include "PlotViewPanel.h"
#include "RpGui.h"
#include <Engine/Events.h>


namespace PH::RpGui {
	
	PlotViewPanel PlotViewPanel::create(Box2D range)  {

		PlotViewPanel result;
		result.display = Engine::createImGuiDisplay(1920, 1080);
		result.region.left = 0.0f;
		result.region.right = (real32)result.display.framebuffersize.x;
		result.region.bottom = 0.0f;
		result.region.top = (real32)result.display.framebuffersize.y;

		result.range = range;

		result.isfocussed = false;

		return result;

	}

	struct FunctionParams {
		real32 cutoff;
		real32 Qfactor;
		real32 gain;
	};

	real32 bandpass(real32 x, void* params_) {

		FunctionParams* params = (FunctionParams*)params_;
		return (x * x + params->cutoff * params->cutoff) / (x * x + ((x * params->cutoff) / params->Qfactor) + params->cutoff * params->cutoff);
	}

	void PlotViewPanel::onEvent(PH::Platform::Event* event) {

		real32 scrollspeed = 0.001f;

		if (event->type == PH_EVENT_TYPE_MOUSE_SCROLLED) {
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
		}
	}

	void PlotViewPanel::ImGuiDraw() {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		auto& io = ImGui::GetIO();

		if (ImGui::Begin("ViewPlot")) {
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
				glm::vec2 mousefactor = { (range.right - range.left) / (region.right - region.left), (range.top - range.bottom) / (region.top - region.bottom) };
				if (PH::Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {

					glm::vec2 deltam = PH::Engine::Events::getDeltaMousePos();

					range.left -= mousefactor.x * deltam.x;
					range.right -= mousefactor.x * deltam.x;
					range.top += mousefactor.y * deltam.y;
					range.bottom += mousefactor.y * deltam.y;
				}
			}
		}

		ImGui::End();

		ImGui::PopStyleVar(1);
	}

	void PlotViewPanel::draw() {

		//begin renderpass for this display
		Engine::beginRenderPass(display);

		PH::RpGui::context->buffer.clear();
		static int32 nsamples = 2000;

		real32 xrange = range.right - range.left;
		real32 dx = xrange / (real32)nsamples;

		static FunctionParams params = { 100.0f, 4.0f, 10.0f };

		//draw the specified function, is going to change in the future to allow for different functions and parameters, for now its just a bandpass filter
		for (real32 x = range.left; x <= range.right; x += dx) {
			glm::vec2 coordinate = { x, 20 * log10(bandpass(powf(10.0f, x), (void*)&params)) };
			PH::RpGui::context->buffer.pushBack(coordinate);
		}

		//start drawing the plot, first set the pipeline and the view and projection matrices, then draw the background and the plot itself, then end the renderer and flush it to the GPU
		RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->pipeline2D);
		RpGui::renderer2D.setView(glm::mat4(1.0f));
		RpGui::renderer2D.setProjection(glm::ortho(0.0f, (real32)region.right, (real32)region.top, 0.0f));
		
		//draw the plot with lines and the plot itself
		drawPlotScaleLines(range, region);
		drawPlot(RpGui::context->buffer.getArray(), range, region);

		static real32 textscale = 0.5f;

		//start drawing the text
		RpGui::renderer2D.pushGraphicsPipeline(RpGui::context->fontpipeline2D, { &RpGui::context->font.cdata, 1 });
		RpGui::renderer2D.pushTexture(RpGui::context->font.atlas);
		RpGui::drawPlotScaleValues(range, region, &RpGui::context->font, textscale);

		RpGui::renderer2D.flush({ nullptr, 0 });

		//end renderpass for this display
		Engine::endRenderPass(display);
	}



}