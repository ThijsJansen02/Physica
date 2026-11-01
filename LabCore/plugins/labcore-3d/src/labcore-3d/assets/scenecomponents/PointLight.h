#pragma once

#include "../../DefferredRenderer.h"
#include "../Scene.h"

namespace PH::LabCore3D {
	
	struct PointLight {

		static const char* getComponentName() {
			return "Point Light";
		}

		static SceneComponentDescription createDescription();

		DefferedRenderer::PointLight light;
	};



}