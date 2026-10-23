#include "SceneView.h"
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Font.h>

#include "../DefferredRenderer.h"

namespace PH::LabCore3D {

	const char* sourceoptions[] = { "albedo", "normal", "lighting" }; 

	PH::bool32 drawSceneView(SceneView* sceneview) {
		
		glm::vec2 viewsize = LabCore::getViewSize();

		//imgui draw part
		real32 titlebarheight = ImGui::GetFrameHeight();
		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		sceneview->display.viewport = { displaysize.x, displaysize.y };


		//should have more consistency between x, y and width and height. This is confusing
		renderer.viewport.width = sceneview->display.viewport.x;
		renderer.viewport.height = sceneview->display.viewport.y;

		setCameraPerspective(&sceneview->camera, 70.0f, sceneview->display.viewport.x / sceneview->display.viewport.y, 0.1f, 100.0f);
		glm::mat4 viewmatrix = calculateCameraViewMatrix(sceneview->camera);


		Engine::AssetLibrary* assets = LabCore::getAssetLibrary();
		auto* cubemesh = assets->getAssetByReference<Mesh>("cubemesh");
		auto* pipe = assets->getAssetByReference<GraphicsPipeline>("defaultmaterialpipe");

		renderer.beginGeometryPass();

		renderer.setProjection(sceneview->camera.perspective);
		renderer.setView(viewmatrix);

		renderer.drawMesh(cubemesh, pipe);

		auto* lightingpipe = assets->getAssetByReference<GraphicsPipeline>("defaultlightingpipe");

		renderer.endGeometryPass();
		renderer.applyLightpass(lightingpipe);
		renderer.output(&sceneview->display, sceneview->displaysrc);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
		if (ImGui::Begin("scene info")) {
			ImGui::Text("camera direction: %f, %f, %f", sceneview->camera.direction.x, sceneview->camera.direction.y, sceneview->camera.direction.z);



			if(ImGui::BeginCombo("renderer output", sourceoptions[sceneview->displaysrc])) {

				for (uint32 i = 0; i < ARRAY_LENGTH(sourceoptions); i ++) {

					if (ImGui::Selectable(sourceoptions[i], i == sceneview->displaysrc)) {
						sceneview->displaysrc = (DefferedRenderer::OutputSource)i;
					}

				}

				ImGui::EndCombo();
			}
		}
		ImGui::PopStyleVar();
		ImGui::End();
		

		
		ImGui::Image(
			sceneview->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / sceneview->display.framebuffersize.x, displaysize.y / sceneview->display.framebuffersize.y } //top right UV
		);

		return true;
	}

	PH::bool32 updateSceneView(SceneView* view) {
		real32 movespeed = 3.0f * PH::Engine::getTimeStep();

		if (LabCore::isViewFocussed()) {
			glm::vec3 move{0.0f};

			if (Engine::Events::isKeyPressed(PH_KEY_W)) {
				move.z += 1.0f;
			}

			if (Engine::Events::isKeyPressed(PH_KEY_S)) {
				move.z -= 1.0f;
			}

			if (Engine::Events::isKeyPressed(PH_KEY_A)) {
				move.x -= 1.0f;
			}

			if (Engine::Events::isKeyPressed(PH_KEY_D)) {
				move.x += 1.0f;
			}

			moveCameraRelative(&view->camera, move * movespeed);

			glm::vec2 dmp = Engine::Events::getDeltaMousePos();

			dmp *= 0.3f;

			if (Engine::Events::isMouseButtonPressed(PH_RMBUTTON)) {
				rotateCameraPitch(&view->camera, dmp.y);
				rotateCameraYaw(&view->camera, dmp.x);
			}
		}

		return true;
	}

	PH::bool32 instantiateSceneView(SceneView* view) {
		view->display = Engine::createImGuiDisplay(1920, 1080);
		view->display.clearcolor = { 0.5f, 0.1f, 0.1f, 1.0f };

		view->camera.direction = { 0.0f, 0.0f, -1.0f };
		view->camera.position = { 0.0f, 0.0f, 5.0f };

		view->displaysrc = DefferedRenderer::OutputSource::DEFFERED_LIGHTING;

		return true;
	}
	
	PH::bool32 destroySceneView(SceneView* data) {

		Engine::destroyDisplay(&data->display);
		return true;
	}

	PH::bool32 SceneViewOnEvent(SceneView* data, const PH::Platform::Event& e) {
		return true;
	}

	PH::bool32 deserializeSceneView(SceneView* data, const YAML::Node& node) {

		if (node["Camera"]) {
			deserializeCamera(&data->camera, node["Camera"]);
		}
		return true;
	}

	PH::bool32 serializeSceneView(SceneView* data, YAML::Emitter& out) {

		out << YAML::BeginMap;

		out << YAML::Key << "Camera" << YAML::Value;
		
		serializeCamera(&data->camera, out);

		out << YAML::EndMap;

		return true;
	}

	LabCore::ViewDescription createSceneViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawSceneView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateSceneView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)SceneViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroySceneView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateSceneView;

		d.windowpadding = { 0,0 };
		d.windowflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeSceneView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeSceneView;

		d.name = Engine::String::create("Scene View");
		d.instancedatasize = sizeof(SceneView);

		return d;
	}
}