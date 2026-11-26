#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "PlotView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/imgui/extensions.h>
#include <Engine/coreassets/Font.h>
#include "../components/Grid.h"

using namespace PH::Platform;

namespace PH::LabCorePLT {

	typedef PlotView View;

	//the global plotview settings, are to be set by the plotview at the beginning of drawing the plot
	PlotView::Settings basesettings{};
	static PlotView::Settings* currentplotviewsettings = &basesettings;

	PlotView::Settings* getCurrentPlotViewSettings() {
		return currentplotviewsettings;
	}

	void drawRenderTest(View* view);

	static PH::bool32 drawView(View* view) {

		currentplotviewsettings = &view->settings;
		
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		view->display.viewport = { displaysize.x, displaysize.y };

		auto* renderer = LabCore::getRenderer2D();


		renderer->viewport.x = 0;
		renderer->viewport.y = 0;
		renderer->viewport.width = displaysize.x;
		renderer->viewport.height = displaysize.y;
		
		view->camera.view = getPlotCameraViewMatrix(view->camera);
		view->camera.proj = getPlotCameraProjectionMatrix(view->camera);
		view->renderprops.camera.domain = view->camera.domain;

		view->renderprops.camera.orthographic = true;
		view->renderprops.camera.view = view->camera.view;
		view->renderprops.camera.projection = view->camera.proj;
		view->renderprops.camera.worldtoscreen = getWorldToScreenSpace(view->camera, view->display.viewport);

		view->renderprops.viewportsize = view->display.viewport;

		renderer->beginGeometryPass(true, true);
		renderer->pushProjection(view->camera.proj);
		renderer->pushView(view->camera.view);

		for (const auto& desc : view->scene.scenecomponentdescriptions) {
			if (desc.drawComponents_) {
				desc.drawComponents_(&view->scene, &view->renderprops);
			}
		}

		renderer->endGeometryPass();
		renderer->output(&view->display);

		//draw the actual rendered image to imgui
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});

		ImGui::ImageButton("###1",
			view->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / view->display.framebuffersize.x, displaysize.y / view->display.framebuffersize.y } //top right UV
		);
		ImGui::PopStyleVar(2);

		if (ImGui::BeginPopupContextItem()) // Uses item's ID and right mouse button by default
		{
			ImGui::SeparatorText("Plot settings");
			ImGui::Checkbox("log x", (bool*) & view->settings.logx);
			ImGui::Checkbox("log y", (bool*)&view->settings.logy);
			ImGui::EndPopup();
		}


		if(ImGui::Begin("render stats")) {
			ImGui::Text("FPS: %f", 1.0f / Engine::getTimeStep());
			ImGui::Text("Delta Mouse: %f, %f", Engine::Events::getDeltaMousePos().x, Engine::Events::getDeltaMousePos().y);
			ImVec2 wp = ImGui::GetWindowPos();
			ImGui::Text("window pos: %f, %f", wp.x, wp.y);
			ImGui::Text("%f", (glm::mat2(view->renderprops.camera.worldtoscreen) * glm::vec2(1, 0)).x);
		}
		ImGui::End();

		return true;
	}

	static PH::bool32 updateView(View* view) {
		if (view->dragging) {
			glm::vec2 deltass = Engine::Events::getDeltaMousePos();
			glm::vec2 deltaws = glm::inverse(glm::mat2(view->renderprops.camera.worldtoscreen)) * deltass;

			view->camera.domain = view->camera.domain + -1.0f * deltaws;
			if (!Engine::Events::isMouseButtonPressed(PH_LMBUTTON) || !LabCore::isViewFocussed()) {	
				view->dragging = false;
			}
		}

		return true;
	}

	static PH::bool32 instantiateView(View* view) {

		view->display = Engine::createImGuiDisplay(2560, 1080);
		LabCore::initScene(&view->scene);

		view->dragging = false;
		view->settings.logx = false;
		view->settings.logy = false;
		view->settings.x_unit = "";
		view->settings.y_unit = "";

		entt::entity ent = LabCore::createEntity(&view->scene);
		LabCore::addComponent<LabCorePLT::Grid2D>(&view->scene, ent, {});

		auto* lib = LabCore::getAssetLibrary();
		lib->addPersistentAsset<Scene>(&view->scene, Engine::createRandomUUID(), "plot");

		view->camera = createPlotCamera(Engine::Box2D::create(-2.0f, 2.0f, 2.0f, -2.0f), { 0.0f, 0.0f });
		return true;
	}

	static PH::bool32 destroyView(View* data) {
		return true;
	}

	real64 scrollspeed = 0.001;

	//transforms the mouseposition to the viewspace
	static glm::vec2 transFormMousePosToDomain(glm::vec2 mousepos, glm::vec2 viewport, Engine::Box2D domain) {
		glm::vec2 r;
		r.x = domain.left + (mousepos.x / viewport.x) * (domain.right - domain.left);
		r.y = domain.bottom + (mousepos.y / viewport.y) * (domain.top - domain.bottom);

		return r;
	}

	static PH::bool32 ViewOnEvent(View* view, const PH::Platform::Event& e) {
		if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			if (e.lparam == PH_LMBUTTON) {
				view->dragging = true;
				glm::vec2 mp = LabCore::getViewMousePos();
				Engine::INFO << "mp: " << mp.x << ", " << mp.y << "\n";
			}
		}
		
		if (e.type == PH_EVENT_TYPE_MOUSE_SCROLLED && LabCore::isViewHovered()) {
			int16 delta = (int16)(void*)e.lparam;

			real32 zoom = 1.0f - (real32)delta * scrollspeed;

			glm::vec2 mp_ss = LabCore::getViewMousePos();
			glm::vec2 windowsize =LabCore::getViewSize();

			glm::vec2 mp_ws = transFormMousePosToDomain(mp_ss, windowsize, view->camera.domain);

			Engine::Box2D range = view->camera.domain;

			//update mouseevents
			if (PH::Engine::Events::isKeyPressed(PH_CONTROL)) {
				range.right = (range.right - mp_ws.x) * zoom + mp_ws.x;
				range.left = (range.left - mp_ws.x) * zoom + mp_ws.x;
			}
			else {
				range.top = (range.top - mp_ws.y) * zoom + mp_ws.y;
				range.bottom = (range.bottom - mp_ws.y) * zoom + mp_ws.y;
			}

			view->camera.domain = range;

			/*
			if (xlock) {
				xlock->range.left = range.left;
				xlock->range.right = range.right;
			}
			*/
		}
		
		return true;
	}

	static PH::bool32 deserializeView(View* data, const YAML::Node& node) {
		instantiateView(data);
		return true;
	}

	static PH::bool32 serializeView(View* data, YAML::Emitter& out) {
		return true;
	}

	LabCore::ViewDescription createPlotViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)ViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroyView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateView;

		d.windowpadding = { 0,0 };
		d.windowflags = 0;

		//d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeView;
		//d.serialize_ = (LabCore::ViewSerializeFunction)serializeView;

		d.name = Engine::String::create("PlotView");
		d.instancedatasize = sizeof(View);

		return d;
	}

	void drawRenderTest(View* view) {

		Engine::BatchRenderer2D* renderer = LabCore::getRenderer2D();

		renderer->drawLine({ 100.0f, 100.0f }, { 500.0f, 100.0f }, 0.5f, 15.0f);
		renderer->drawCircle({ 100.0f, 100.0f, 1.0f }, 0.0f, { 10.0f, 10.0f }, { 1.0f, 0.0f, 1.0f, 1.0f });
		renderer->drawCircle({ 700.0f, 500.0f, 1.0f }, 0.0f, { 100.0f, 100.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
		renderer->drawRectangle({ 100.0, 200.0, 2.0 }, 0.0, { 100.0f, 100.0f }, { 1.0f, 0.0, 0.0, 1.0 });

		auto* font = LabCore::getAssetLibrary()->getLoadedAssetByReference<Engine::Font>("defaultfont");
		renderer->drawText("Hello world! I want to show you some more text", { 200.0f, 200.0f, 0.0f }, 400.0f, font);
	
	
	}

}