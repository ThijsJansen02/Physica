#pragma once

#include "../../DefferredRenderer.h"
#include <LabCore-scene/assets/Scene.h>

namespace PH::LabCore3D {
	
	struct PointLight {

		static const char* getComponentName() {
			return "Point Light";
		}

		static LabCore::SceneComponentDescription createDescription();

		DefferedRenderer::PointLight light;
	};



}