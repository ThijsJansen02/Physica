#include "Camera.h"

//#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <Engine/YamlExtensions.h>


namespace PH::LabCore3D {

	void setCameraPerspective(Camera* camera, real32 fov, real32 aspectratio, real32 nearplane, real32 farplane) {
		glm::mat4 invert_y = glm::mat4({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });

		camera->perspective = invert_y * glm::perspectiveLH(glm::radians(fov), aspectratio, nearplane, farplane);
	}

	glm::mat4 calculateCameraViewMatrix(const Camera& camera) {
		return glm::lookAtLH(camera.position, camera.position + camera.direction, { 0.0f, 1.0f, 0.0f });
	}

	void moveCameraRelative(Camera* camera, glm::vec3 translation) {
		glm::vec3 negtangent = -1.0f * glm::normalize(glm::cross(camera->direction, { 0.0f, 1.0f, 0.0f }));

		camera->position += camera->direction * translation.z;
		camera->position += negtangent * translation.x;
	}

	void rotateCameraPitch(Camera* camera, real32 pitch) {

		glm::vec3 negtangent = -1.0f * glm::normalize(glm::cross(camera->direction, { 0.0f, 1.0f, 0.0f }));
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), -negtangent);

		camera->direction = rotate * glm::vec4(camera->direction, 1.0f);

		camera->direction = glm::normalize(camera->direction);
	}

	void rotateCameraYaw(Camera* camera, real32 yaw) {
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

		camera->direction = rotate * glm::vec4(camera->direction, 1.0f);

		camera->direction = glm::normalize(camera->direction);
	}

	void rotateCamera(Camera* camera, real32 pitch, real32 yaw) {

		rotateCameraYaw(camera, yaw);
		rotateCameraPitch(camera, pitch);
	}

	void serializeCamera(Camera* camera, YAML::Emitter& out) {
		out << YAML::BeginMap;

		out << YAML::Key << "direction" << YAML::Value << camera->direction;
		out << YAML::Key << "position" << YAML::Value << camera->position;

		out << YAML::EndMap;
	}

	void deserializeCamera(Camera* camera, const YAML::Node& node) {

		if (node["direction"]) {
			camera->direction = node["direction"].as<glm::vec3>();
		}

		if (node["position"]) {
			camera->position = node["position"].as<glm::vec3>();
		}

	}

}