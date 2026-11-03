#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/Display.h>
#include "../DefferredRenderer.h"

#include "../Camera.h"

#include "../imguizmo/imguizmoinclude.h"

namespace PH::LabCore3D {

	struct SceneView {

		Camera camera;

		ImGuizmo::OPERATION operation;

		bool32 pickmouseobject;
		Engine::DynamicGPUBuffer<uint32> pixelreadbuffer;

		GraphicsPipeline* tonemappipe;
		GraphicsPipeline* lightingpipe;

		DefferedRenderer::OutputSource displaysrc;
		Engine::AssetIdentifier* scene;

		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createSceneViewDescription();
}