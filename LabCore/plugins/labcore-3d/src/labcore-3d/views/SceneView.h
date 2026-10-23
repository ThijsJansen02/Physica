#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/Display.h>

#include "../Camera.h"

namespace PH::LabCore3D {

	struct SceneView {

		Camera camera;

		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createSceneViewDescription();
}