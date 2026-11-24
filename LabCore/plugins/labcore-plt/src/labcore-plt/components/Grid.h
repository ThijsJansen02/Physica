#pragma once
#include <Engine/Engine.h>
#include <labcore-scene/assets/scene.h>

namespace PH::LabCorePLT {

	struct Grid2D {
		
		static const char* getComponentName() {
			return "2D Grid PLT";
		}

		static LabCore::SceneComponentDescription createDescription();
	};
}

