#pragma once

#include <Engine/Engine.h>
#include "../Scene.h"

namespace PH::LabCore3D {

	struct Transform {
		
		//the transform calculated from the euler angles and values
		glm::mat4 transform;

		glm::vec3 rotXYZ;
		glm::vec3 scale;
		glm::vec3 translation;

		static const char* getComponentName() {
			return "Transform";
		}

		static SceneComponentDescription createDescription();
	};
}
