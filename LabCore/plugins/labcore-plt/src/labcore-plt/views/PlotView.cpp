
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "PlotView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/imgui/extensions.h>
#include <Engine/coreassets/Font.h>
#include "../components/Grid.h"

using namespace PH::Platform;

namespace PH::LabCorePLT {

	typedef PlotView View;

	void drawRenderTest(View* view);

	static PH::bool32 drawView(View* view) {
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		view->display.viewport = { displaysize.x, displaysize.y };

		auto* renderer = LabCore::getRenderer2D();


		renderer->viewport.x = 0;
		renderer->viewport.y = 0;
		renderer->viewport.width = displaysize.x;
		renderer->viewport.height = displaysize.y;
		


		view->camera.view = getPlotCameraViewMatrix(view->camera);
		view->camera.proj = getPlotCameraProjectionMatrix(view->camera);

		view->scene.transforms.orthographic = true;
		view->scene.transforms.view = view->camera.view;
		view->scene.transforms.projection = view->camera.proj;
		view->scene.transforms.worldtoscreen = getWorldToScreenSpace(view->camera, view->display.viewport);

		renderer->beginGeometryPass(true, true);
		renderer->pushProjection(view->camera.proj);
		renderer->pushView(view->camera.view);

		for (const auto& desc : view->scene.scenecomponentdescriptions) {
			if (desc.drawComponents_) {
				desc.drawComponents_(&view->scene);
			}
		}

		renderer->endGeometryPass();
		renderer->output(&view->display);

		//draw the actual rendered image to imgui
		ImGui::Image(
			view->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / view->display.framebuffersize.x, displaysize.y / view->display.framebuffersize.y } //top right UV
		);

		if(ImGui::Begin("render stats")) {
			ImGui::Text("FPS: %f", 1.0f / Engine::getTimeStep());
			ImGui::Text("Delta Mouse: %f, %f", Engine::Events::getDeltaMousePos().x, Engine::Events::getDeltaMousePos().y);
		}
		ImGui::End();

		return true;
	}

	static PH::bool32 updateView(View* view) {
		if (view->dragging) {
			glm::vec2 deltass = Engine::Events::getDeltaMousePos();
			glm::vec2 deltaws = glm::inverse(glm::mat2(view->scene.transforms.worldtoscreen)) * deltass;

			view->camera.domain = view->camera.domain + -1.0f * deltaws;
			if (!Engine::Events::isMouseButtonPressed(PH_LMBUTTON)) {
				view->dragging = false;
			}
		}

		return true;
	}

	static PH::bool32 instantiateView(View* view) {

		view->display = Engine::createImGuiDisplay(2560, 1080);
		LabCore::initScene(&view->scene);

		view->dragging = false;

		entt::entity ent = LabCore::createEntity(&view->scene);
		LabCore::addComponent<LabCorePLT::Grid2D>(&view->scene, ent, {});

		auto* lib = LabCore::getAssetLibrary();
		lib->addPersistentAsset<Scene>(&view->scene, Engine::createRandomUUID(), "plot");

		view->camera = createPlotCamera(Engine::Box2D::create(-10.0f, 10.0f, 10.0f, -10.0f), { 0.0f, 0.0f });
		return true;
	}

	static PH::bool32 destroyView(View* data) {
		return true;
	}

	static PH::bool32 ViewOnEvent(View* view, const PH::Platform::Event& e) {
		if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			if (e.lparam == PH_LMBUTTON) {
				view->dragging = true;
			}
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