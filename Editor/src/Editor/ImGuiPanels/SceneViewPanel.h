#pragma once
#include <Engine/Display.h>
#include "../EditorCamera.h"
#include "../Editor.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <Engine/Components.h>

#include "../EditorAPI.h"
#include <ImGuizmo.h>

namespace PH::Editor {

	struct SceneViewCreate {
		uint32 maxwidth;
		uint32 maxheight;
		EditorCamera camera;
	};

	struct SceneView {
		Engine::ImGuiDisplay display;
		EditorCamera camera;

		bool32 focussed;

		int gizmotype = ImGuizmo::OPERATION::ROTATE;
	};

	inline SceneView createSceneView(const SceneViewCreate& create) {
		SceneView result;
		result.display = Engine::createImGuiDisplay(create.maxwidth, create.maxheight);
		result.camera = create.camera;

		return result;
	}

	inline void sceneViewOnEvent(PH::Platform::Event e, Editor::SceneView* view) {
		if (e.type == PH_EVENT_TYPE_KEY_PRESSED) {

			bool32 control = Engine::Events::isKeyPressed(PH_CONTROL);

			if (control) {
				switch (e.lparam) {
				case PH_KEY_R:
					view->gizmotype = ImGuizmo::OPERATION::ROTATE;
					break;

				case PH_KEY_Q:
					view->gizmotype = ImGuizmo::OPERATION::TRANSLATE;
					break;

				case PH_KEY_E:
					view->gizmotype = ImGuizmo::OPERATION::SCALE;
					break;
				}

			}

		}
	}

	inline bool32 drawSceneToSceneView(
		Engine::Assets::Scene::Instance* instance,
		Engine::Renderer2D::Context* renderer2d,
		Engine::Renderer3D::Context* renderer3d,
		Editor::SceneView* view
	) {

		//maybe only update when an actual input event happend
		updateCameraMatrices(&view->camera);

		Engine::beginRenderPass(view->display);
		Engine::Assets::Scene::render(renderer2d, renderer3d, instance, view->camera.cam);
		Engine::endRenderPass(view->display);

		return true;
	}

	//determines the viewport of the display display and issues commmands to imgui to draw a sceneviewpanel
	inline bool32 drawSceneViewPanel(Editor::SceneView* sceneview, Engine::Assets::Scene::Entity selectedentity) {
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

		//get the editor
		Editor::Instance* editor = Editor::API::getCurrentInstance();

		//update the camera
		Editor::updateCamera(&sceneview->camera, Engine::getTimeStep());

		sceneview->focussed = ImGui::IsWindowFocused();

		ImVec2 displaysize = ImGui::GetContentRegionAvail();
		sceneview->display.viewport = { displaysize.x, displaysize.y };
		sceneview->camera.aspectratio = Engine::getDisplayAspectRatio(sceneview->display);

		ImGui::Image(
			sceneview->display.imguitexture, 
			displaysize, 
			{ 0.0f, 0.0f }, //bottom left uv
			{ displaysize.x / sceneview->display.framebuffersize.x, displaysize.y / sceneview->display.framebuffersize.y } //top right UV
		);

		//figure out the viewport bounds
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
			
		ImVec2 viewportbounds[2];
		viewportbounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		viewportbounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		glm::mat4 projectioncorrection = glm::transpose(glm::mat4(
			glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f,  1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
		));

		// Gizmos
		if (selectedentity.valid() && sceneview->gizmotype != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(viewportbounds[0].x, viewportbounds[0].y, viewportbounds[1].x - viewportbounds[0].x, viewportbounds[1].y - viewportbounds[0].y);

			// Camera

			// Runtime camera from entity
			// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			// const glm::mat4& cameraProjection = camera.GetProjection();
			// glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = projectioncorrection * sceneview->camera.cam.projection;
			glm::mat4 cameraView = sceneview->camera.cam.view;

			// Entity transform
			auto* tc = selectedentity.getComponent<PH::Engine::TransformComponent>();
			glm::mat4 transform = tc->global;

			// Snapping
			bool snap = Engine::Events::isKeyPressed(PH_CONTROL);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (sceneview->gizmotype == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)sceneview->gizmotype, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			tc->global = transform;
		}

		ImGui::PopStyleVar();


		Editor::drawSceneToSceneView(editor->activescene->instance, editor->renderer2d, editor->renderer3d, sceneview);
		return true;
	}

	inline bool32 sceneView_init(void* userdata) {

		SceneView* view = (SceneView*)userdata;

		SceneViewCreate create{};
		create.camera = createPerspectiveEditorCamera(1.0f);
		create.maxwidth = 1920;
		create.maxheight = 1080;

		*view = createSceneView(create);
		return true;
	}

	inline bool32 sceneView_draw(void* userdata) {
		SceneView* view = (SceneView*)userdata;

		Editor::Instance* editor = Editor::API::getCurrentInstance();

		return drawSceneViewPanel(view, editor->selected);
	}

	inline bool32 sceneView_dispatchEvent(void* userdata, const Platform::Event& e) {
		SceneView* view = (SceneView*)userdata;

		sceneViewOnEvent(e, view);
		return true;
	}

	inline bool32 SceneView_serialize(void* userdata, YAML::Emitter& out) {
		
		SceneView* view = (SceneView*)userdata;

		//the sceneview
		out << YAML::BeginMap; //view 1: sceneview
		out << YAML::Key << "Type" << YAML::Value << "SCENE_VIEW";

		out << YAML::Key << "EditorCamera" << YAML::Value << YAML::BeginMap; //editor camera
		out << YAML::Key << "Position" << YAML::Value << view->camera.position;
		out << YAML::Key << "Direction" << YAML::Value << view->camera.direction;
		out << YAML::Key << "Perspective" << YAML::Value << (bool)view->camera.perspective;

		if (view->camera.perspective) {
			out << YAML::Key << "FOV" << YAML::Value << view->camera.FOV;
		}
		else {
			out << YAML::Key << "Zoomx" << YAML::Value << view->camera.zoomx;
			out << YAML::Key << "zoomy" << YAML::Value << view->camera.zoomy;
		}

		out << YAML::Key << "Farclip" << YAML::Value << view->camera.farclip;
		out << YAML::Key << "Nearclip" << YAML::Value << view->camera.nearclip;

		out << YAML::EndMap; //editorcamera
		out << YAML::EndMap; //view 1: sceneview

		return true;
	}

	inline bool32 SceneView_deserialize(void* userdata, YAML::Node& root) {
	
		SceneView* view = (SceneView*)userdata;

		SceneViewCreate create{};
		create.maxheight = 1080;
		create.maxwidth = 1920;

		*view = createSceneView(create);

		auto cam = root["EditorCamera"];

		view->camera.perspective = cam["Perspective"].as<bool>();

		if (view->camera.perspective) {
			view->camera.FOV = cam["FOV"].as<real32>();
		}
		else {
			view->camera.zoomx = cam["Zoomx"].as<real32>();
			view->camera.zoomy = cam["Zoomy"].as<real32>();
		}

		view->camera.position = cam["Position"].as<glm::vec3>();
		view->camera.direction = cam["Direction"].as<glm::vec3>();

		view->camera.nearclip = cam["Nearclip"].as<real32>();
		view->camera.farclip = cam["Farclip"].as<real32>();

		return true;
	}

#define SCENEVIEW_ID 2

	inline Editor::View loadSceneView() {

		Editor::View sceneview{};
		sceneview.id = Engine::Assets::Library::createRandomUUID();
		sceneview.sourceid = SCENEVIEW_ID;
		sceneview.userdata = Engine::Allocator::alloc(sizeof(Editor::SceneView));
		sceneview.dispatchEvent = Editor::sceneView_dispatchEvent;
		sceneview.draw = Editor::sceneView_draw;
		sceneview.init = Editor::sceneView_init;
		sceneview.name = Engine::String::create("SceneView");
		sceneview.deserialize = Editor::SceneView_deserialize;
		sceneview.serialize = Editor::SceneView_serialize;

		return sceneview;

	}

}