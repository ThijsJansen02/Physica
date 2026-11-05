#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Font.h>
#include "../assets/Scene.h"
#include <Engine/imgui/extensions.h>
#include "../DefferredRenderer.h"
#include "../assets/scenecomponents/Transform.h"
#include "../assets/scenecomponents/NameTag.h"

#include "SceneView.h"
#include "../imguizmo/imguizmoinclude.h"

namespace PH::LabCore3D {

	const char* sourceoptions[] = { "albedo", "normal", "lighting", "position"};

	bool32 geometryPass(SceneView* sceneview, const glm::mat4& projection, const glm::mat4& view) {
	

		renderer.beginGeometryPass();

		renderer.setProjection(projection);
		renderer.setView(view);

		auto* assets = LabCore::getAssetLibrary();
		if (sceneview->scene) {
			if (sceneview->scene->status != Engine::LOADED) {
				assets->loadAsset<Scene>(sceneview->scene->assetid);
			}

			Scene* scene = (Scene*)sceneview->scene->assetdata;

			for (auto& desc : Scene::scenecomponentdescriptions) {
				if (desc.drawComponents_) {
					desc.drawComponents_(scene, &renderer);
				}
			}
		}

		renderer.endGeometryPass();
		return true;
	}

	void lightPass(SceneView* sceneview) {

		auto* assets = LabCore::getAssetLibrary();
		if (sceneview->lightingpipe) {
			renderer.applyLightpass(sceneview->lightingpipe);
		}

	}

	entt::entity aquireObjectID(SceneView* sceneview, glm::vec2 displaysize) {


		glm::vec2 mousepos = LabCore::getViewMousePos();

		mousepos.y = displaysize.y - mousepos.y;

		mousepos.x = Base::clamp(mousepos.x, 0.0f, displaysize.x);
		mousepos.y = Base::clamp(mousepos.y, 0.0f, displaysize.y);

		Platform::GFX::ImageToBufferCopyInfo copyinfo;
		copyinfo.bufferoffset = 0;
		copyinfo.dst = sceneview->pixelreadbuffer.buffer;
		copyinfo.src = renderer.gbuffer.objectid;
		copyinfo.imageextent = { 1, 1, 1 };
		copyinfo.imageoffset = { (int32)mousepos.x, (int)mousepos.y, 0 };

		PH::Platform::GFX::copyImageToBufferAsynchronous(copyinfo);

		return (entt::entity)*sceneview->pixelreadbuffer.map();
	}

	void drawSceneViewInfo(SceneView* sceneview) {

		glm::vec2 mousepos = LabCore::getViewMousePos();
		auto* assets = LabCore::getAssetLibrary();

		//imgui draw part
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
		if (ImGui::Begin("scene info")) {
			ImGui::Text("camera direction: %f, %f, %f", sceneview->camera.direction.x, sceneview->camera.direction.y, sceneview->camera.direction.z);

			entt::entity hovered = (entt::entity)*sceneview->pixelreadbuffer.map();
			const char* entityname = "no entity";

			if (sceneview->scene) {
				if (sceneview->scene->status == Engine::LOADED) {
					Scene* scene = (Scene*)sceneview->scene->assetdata;

					if (scene->registry.valid(hovered)) {
						if (hasComponent<Tag>(scene, hovered)) {
							entityname = getComponent<Tag>(scene, hovered).name.getC_Str();
						}
					}
				}
			}

			ImGui::Text("mousepos: %f, %f, object at position: %s, objectid: %u", mousepos.x, mousepos.y, entityname, *sceneview->pixelreadbuffer.map());
			ImGui::Text("FPS: %f", 1.0f / Engine::getTimeStep());

			sceneview->scene = ImGuiext::assetSelecter<Scene>("scene", sceneview->scene, "select scene", assets);

			if (ImGui::BeginCombo("renderer output", sourceoptions[sceneview->displaysrc])) {

				for (uint32 i = 0; i < ARRAY_LENGTH(sourceoptions); i++) {

					if (ImGui::Selectable(sourceoptions[i], i == sceneview->displaysrc)) {
						sceneview->displaysrc = (DefferedRenderer::OutputSource)i;
					}

				}

				ImGui::EndCombo();
			}

			sceneview->tonemappipe = ImGuiext::assetSelecter<GraphicsPipeline>("tonemap pipeline", sceneview->tonemappipe, "select tonemap pipeline", assets);
			sceneview->lightingpipe = ImGuiext::assetSelecter<GraphicsPipeline>("lighting pipeline", sceneview->lightingpipe, "select lighting pipeline", assets);
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}

	void drawImGuizmo(SceneView* sceneview, glm::vec2 displaysize, glm::mat4 projection, const glm::mat4& view) {
		//draw the guizmo
		ImGuizmo::BeginFrame();

		auto* assets = LabCore::getAssetLibrary();

		if (sceneview->scene) {
			Scene* scene = assets->getLoadedAsset<Scene>(sceneview->scene->assetid);

			if (scene->selected != entt::null) {
				
				if (hasComponent<Transform>(scene, scene->selected)) {
					ImGuizmo::SetOrthographic(false);
					//ImGuizmo::SetDrawlist();

					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(0.0f, 0.0f, displaysize.x, displaysize.y);

					projection[1][1] *= -1.0f;

					Transform& tc = getComponent<Transform>(scene, scene->selected);

					// Snapping
					bool snap = Engine::Events::isKeyPressed(PH_CONTROL);
					float snapValue = 0.5f; // Snap to 0.5m for translation/scale
					// Snap to 45 degrees for rotation

					if (sceneview->operation == ImGuizmo::OPERATION::ROTATE)
						snapValue = 45.0f;

					float snapValues[3] = { snapValue, snapValue, snapValue };

					ImGuizmo::Manipulate((real32*)&view, (real32*)&projection,
						sceneview->operation, ImGuizmo::LOCAL, (real32*)&tc.transform,
						nullptr, snap ? snapValues : nullptr);
					
					if (ImGuizmo::IsUsing())
					{
						glm::vec3 translation, rotation, scale;
						ImGuizmo::DecomposeMatrixToComponents((real32*)&tc.transform, (real32*)&tc.translation, (real32*)&tc.rotXYZ, (real32*)&tc.scale);
					}
				}
			}
		}
	}

	PH::bool32 drawSceneView(SceneView* sceneview) {
		
		auto* assets = LabCore::getAssetLibrary();

		drawSceneViewInfo(sceneview);

		if (!sceneview->scene) {
			return true;
		}

		if (sceneview->scene->status != Engine::LOADED) {
			Platform::setCurrentDirectory(LabCore::getCurrentProjectDir().getC_Str());
			//require the scene to be loaded
			sceneview->scene = assets->getLoadedAssetIdentifier<Scene>(sceneview->scene->assetid);
		}

		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		sceneview->display.viewport = { displaysize.x, displaysize.y };

		glm::mat4 projection = calculateCameraPerspective(&sceneview->camera, 70.0f, sceneview->display.viewport.x / sceneview->display.viewport.y, 0.1f, 100.0f);
		glm::mat4 viewmatrix = calculateCameraViewMatrix(sceneview->camera);

		//should have more consistency between x, y and width and height. This is confusing
		renderer.viewport.width = sceneview->display.viewport.x;
		renderer.viewport.height = sceneview->display.viewport.y;

		geometryPass(sceneview, projection, viewmatrix);

		lightPass(sceneview);

		if (sceneview->tonemappipe) {
			renderer.output(&sceneview->display, sceneview->tonemappipe, sceneview->displaysrc);
		}

		entt::entity hoveredentity = aquireObjectID(sceneview, {displaysize.x, displaysize.y});
		
		//draw the actual rendered image to imgui
		ImGui::Image(
			sceneview->display.imguitexture,
			displaysize,
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / sceneview->display.framebuffersize.x, displaysize.y / sceneview->display.framebuffersize.y } //top right UV
		);

		drawImGuizmo(sceneview, { displaysize.x, displaysize.y }, projection, viewmatrix);

		if (!ImGuizmo::IsUsing()) {
			if (sceneview->pickmouseobject) {
				Scene* scene = LabCore::getAssetLibrary()->getLoadedAsset<Scene>(sceneview->scene->assetid);
				scene->selected = (entt::entity)*sceneview->pixelreadbuffer.map();
			}
		}

		sceneview->pickmouseobject = false;

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

				view->camera.direction = glm::normalize(view->camera.direction);
			}
		}

		return true;
	}

	PH::bool32 instantiateSceneView(SceneView* view) {
		view->display = Engine::createImGuiDisplay(1920, 1080);
		view->display.clearcolor = { 0.0f, 0.0f, 0.0f, 1.0f };

		view->pixelreadbuffer = Engine::DynamicGPUBuffer<uint32>::create(1, Platform::GFX::BUFFER_USAGE_TRANSFER_DST_BIT);

		view->camera.direction = { 0.0f, 0.0f, -1.0f };
		view->camera.position = { 0.0f, 0.0f, 5.0f };

		view->tonemappipe = nullptr;
		view->lightingpipe = nullptr;

		view->operation = ImGuizmo::OPERATION::TRANSLATE;
		view->scene = nullptr;

		view->displaysrc = DefferedRenderer::OutputSource::DEFFERED_LIGHTING;

		return true;
	}
	
	PH::bool32 destroySceneView(SceneView* data) {

		Engine::destroyDisplay(&data->display);
		return true;
	}

	PH::bool32 SceneViewOnEvent(SceneView* data, const PH::Platform::Event& e) {
		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {
			if (e.lparam == PH_KEY_R) {
				data->operation = ImGuizmo::ROTATE;
			}

			if (e.lparam == PH_KEY_T) {
				data->operation = ImGuizmo::SCALE;
			}

			if (e.lparam == PH_KEY_G) {
				data->operation = ImGuizmo::TRANSLATE;
			}
		}

		if (e.type == PH_EVENT_TYPE_MOUSEBUTTON_PRESSED) {
			if (e.lparam == PH_LMBUTTON) {
				data->pickmouseobject = true;
			}

		}

		return true;
	}

	PH::bool32 deserializeSceneView(SceneView* data, const YAML::Node& node) {

		if (node["Camera"]) {
			deserializeCamera(&data->camera, node["Camera"]);
		}
		data->scene = nullptr;
		if (node["scene"]) {
			data->scene = LabCore::getAssetLibrary()->getLoadedAssetIdentifier<Scene>(node["scene"].as<Engine::AssetID>());
		}

		auto* assets = LabCore::getAssetLibrary();

		if (node["lightpipe"]) {
			data->lightingpipe = assets->getLoadedAsset<GraphicsPipeline>(node["lightpipe"].as<Engine::AssetID>());
		}

		if (node["tonemappipe"]) {
			data->tonemappipe = assets->getLoadedAsset<GraphicsPipeline>(node["tonemappipe"].as<Engine::AssetID>());
		}

		return true;
	}

	PH::bool32 serializeSceneView(SceneView* data, YAML::Emitter& out) {

		out << YAML::BeginMap;
		if (data->scene) {
			out << YAML::Key << "scene" << YAML::Value << data->scene->assetid;
		}

		out << YAML::Key << "Camera" << YAML::Value;
		serializeCamera(&data->camera, out);
		
		if (data->lightingpipe) {
			out << YAML::Key << "lightpipe" << YAML::Value << data->lightingpipe->id;
		}

		if (data->tonemappipe) {
			out << YAML::Key << "tonemappipe" << YAML::Value << data->tonemappipe->id;
		}


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
		d.windowflags = ImGuiWindowFlags_NoScrollbar;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeSceneView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeSceneView;

		d.name = Engine::String::create("Scene View");
		d.instancedatasize = sizeof(SceneView);

		return d;
	}
}