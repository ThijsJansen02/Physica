#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Texture.h>
#include <labcore-scene/assets/scene.h>
#include "PlotCamera.h"

using namespace PH;
using namespace PH::LabCore;

namespace PH::LabCorePLT {



	struct PlotView {
		PlotCamera camera;

		bool32 dragging;

		Scene scene;
		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createPlotViewDescription();
}