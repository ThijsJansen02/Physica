#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/Display.h>
#include "../DefferredRenderer.h"

#include "../Camera.h"

namespace PH::LabCore3D {

	struct SceneView {

		Camera camera;

		DefferedRenderer::OutputSource displaysrc;

		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createSceneViewDescription();
}