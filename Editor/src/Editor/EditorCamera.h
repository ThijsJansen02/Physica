#pragma once
#include "Engine/Engine.h"
#include "Engine/Events.h"

namespace PH::Editor {

	struct EditorCamera {

		union {
			struct {
				real32 zoomx, zoomy;
			};
			struct {
				real32 FOV;
			};
		};

		real32 aspectratio;
		bool32 perspective;

		real32 farclip;
		real32 nearclip;

		glm::vec3 direction;
		glm::vec3 position;

		Engine::Assets::Scene::Camera cam;
	};

	inline void updateCameraMatrices(EditorCamera* camera) {

		if (!camera->perspective) {
			real32 left = -1.0f * camera->aspectratio * camera->zoomx;
			real32 right = 1.0f * camera->aspectratio * camera->zoomx;
			real32 top = 1.0f * camera->zoomy;
			real32 bottom = -1.0f * camera->zoomy;

			camera->cam.projection = glm::ortho(left, right, bottom, top, camera->nearclip, camera->farclip);
			camera->cam.view = glm::lookAt(camera->position, camera->position + camera->direction, { 0.0f, 1.0f, 0.0f });
		}

		if (camera->perspective) {
			camera->cam.projection = glm::perspective(camera->FOV, camera->aspectratio, camera->nearclip, camera->farclip);
			camera->cam.view = glm::lookAt(camera->position, camera->position + camera->direction, { 0.0f, 1.0f, 0.0f });
		}
	}

	inline void updateCamera(EditorCamera* camera, real32 deltatime) {

		real32 cameraspeed = 1.0f;
		real32 mousesensitivity = 0.5f;

		if (Engine::Events::isKeyPressed(PH_SHIFT)) {
			cameraspeed = 5.0f;
		}

		glm::vec3 left = glm::normalize(glm::cross(camera->direction, { 0.0f, 1.0f, 0.0 }));

		glm::mat3 dyaw = glm::rotate(
			glm::mat4(1.0f),
			glm::radians(Engine::Events::getDeltaMousePos().x) * mousesensitivity,
			glm::vec3{ 0.0f, 1.0f, 0.0f }
		);

		glm::mat3 dpitch = glm::rotate(
			glm::mat4(1.0f),
			glm::radians(Engine::Events::getDeltaMousePos().y) * mousesensitivity,
			-left);

		if (Engine::Events::isMouseButtonPressed(PH_RMBUTTON)) {
			camera->direction = dyaw * dpitch * camera->direction;
		}

		if (Engine::Events::isKeyPressed(PH_KEY_W)) {
			camera->position += camera->direction * cameraspeed * deltatime;
		}

		if (Engine::Events::isKeyPressed(PH_KEY_S)) {
			camera->position -= camera->direction * cameraspeed * deltatime;
		}

		if (Engine::Events::isKeyPressed(PH_KEY_A)) {
			camera->position += left * cameraspeed * deltatime;
		}

		if (Engine::Events::isKeyPressed(PH_KEY_D)) {
			camera->position -= left * cameraspeed * deltatime;
		}
	}

	inline EditorCamera createOrthoEditorCamera(real32 aspectratio) {

		EditorCamera result;
		result.perspective = false;
		result.zoomx = 1.0f;
		result.zoomy = 1.0f;
		result.aspectratio = aspectratio;
		result.nearclip = 0.0f;
		result.farclip = 2.0f;

		result.position = { 0.0f, 0.0f, 0.0f };
		result.direction = { 0.0f, 0.0f, 1.0f };

		updateCameraMatrices(&result);
		return result;
	}

	inline EditorCamera createPerspectiveEditorCamera(real32 aspectratio) {

		EditorCamera result;
		result.perspective = true;
		result.aspectratio = aspectratio;
		result.nearclip = 0.1f;
		result.farclip = 100.0f;
		result.FOV = 70.0f;

		result.position = { 0.0f, 0.0f, -3.0f };
		result.direction = { 0.0f, 0.0f, 1.0f };

		updateCameraMatrices(&result);
		return result;
	}
}