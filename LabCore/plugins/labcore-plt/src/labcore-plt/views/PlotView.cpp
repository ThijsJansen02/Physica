
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "PlotView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/imgui/extensions.h>
#include <Engine/coreassets/Font.h>

using namespace PH::Platform;

namespace PH::LabCorePLT {

	typedef PlotView View;

	static PH::bool32 drawView(View* view) {
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		view->display.viewport = { displaysize.x, displaysize.y };

		auto* renderer = LabCore::getRenderer2D();

		renderer->viewport.x = 0;
		renderer->viewport.y = 0;
		renderer->viewport.width = displaysize.x;
		renderer->viewport.height = displaysize.y;

		glm::mat4 projection = glm::ortho(0.0f, displaysize.x, 0.0f, displaysize.y, -100.0f, 100.0f);


		renderer->beginGeometryPass(true, true);
		renderer->pushProjection(projection);

		renderer->drawLine({ 100.0f, 100.0f }, { 500.0f, 100.0f }, 0.5f, 15.0f);
		renderer->drawCircle({ 100.0f, 100.0f, 1.0f }, 0.0f, { 10.0f, 10.0f }, { 1.0f, 0.0f, 1.0f, 1.0f });
		renderer->drawCircle({ 700.0f, 500.0f, 1.0f }, 0.0f, { 100.0f, 100.0f }, { 0.0f, 1.0f, 0.0f, 1.0f });
		renderer->drawRectangle({ 100.0, 200.0, 2.0 }, 0.0, { 100.0f, 100.0f }, {1.0f, 0.0, 0.0, 1.0});
		
		auto* font = LabCore::getAssetLibrary()->getLoadedAssetByReference<Engine::Font>("defaultfont");

		renderer->drawRectangle({ 400.0f, 400.0f, 3.0f }, 0.0f, { 280, 280}, font->atlas.texture);

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
		}
		ImGui::End();

		return true;
	}

	static PH::bool32 updateView(View* view) {


		return true;
	}

	static PH::bool32 instantiateView(View* view) {
		Platform::setCurrentDirectory(LabCore::getHomeDirectory());

		view->display = Engine::createImGuiDisplay(2560, 1080);
		view->texture = Engine::createTexture("res/textures/checkerboard.jpg", GFX::FORMAT_R8G8B8A8_SRGB);
		return true;

		Platform::setCurrentDirectory(LabCore::getCurrentProjectDir());
	}

	static PH::bool32 destroyView(View* data) {
		return true;
	}

	static PH::bool32 ViewOnEvent(View* data, const PH::Platform::Event& e) {
		return true;
	}

	static PH::bool32 deserializeView(View* data, const YAML::Node& node) {
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
}