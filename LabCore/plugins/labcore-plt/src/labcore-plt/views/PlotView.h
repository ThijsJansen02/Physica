#pragma once
#include <LabCore/LabCoreAPI.h>
#include <Engine/coreassets/Texture.h>

using namespace PH;
using namespace PH::LabCore;

namespace PH::LabCorePLT {

	struct PlotView {
		Engine::Texture texture;

		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createPlotViewDescription();
}