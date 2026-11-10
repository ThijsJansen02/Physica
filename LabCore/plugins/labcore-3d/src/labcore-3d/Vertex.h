#pragma once
#include <glm/glm.hpp>
namespace PH::LabCore3D {

	struct Vertex3D {
		glm::vec3 position;
		glm::vec2 texcoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

}