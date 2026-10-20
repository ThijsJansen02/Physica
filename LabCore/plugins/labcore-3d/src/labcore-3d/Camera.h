#pragma once
#include <glm/glm.hpp>
#include <Base/base.h>
#include <yaml-cpp/yaml.h>

namespace PH::LabCore3D {

	struct Camera {
		glm::vec3 position;
		glm::vec3 direction;

		glm::mat4 perspective;
	};
	
	void setCameraPerspective(Camera* camera, real32 fov, real32 aspectratio, real32 nearplane, real32 farplane);
	glm::mat4 calculateCameraViewMatrix(const Camera& camera);

	void moveCameraRelative(Camera* camera, glm::vec3 translation);
	void rotateCamera(Camera* camera, real32 pitch, real32 yaw);

	void rotateCameraPitch(Camera* camera, real32 pitch);
	void rotateCameraYaw(Camera* camera, real32 yaw);

	void serializeCamera(Camera* camera, YAML::Emitter& out);
	void deserializeCamera(Camera* camera, const YAML::Node& node);


}

