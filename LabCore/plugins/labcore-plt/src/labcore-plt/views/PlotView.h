#pragma once
#include <LabCore/LabCoreAPI.h>

using namespace PH;
using namespace PH::LabCore;

namespace PH::LabCorePLT {

	struct PlotView {
		Engine::ImGuiDisplay display;
	};

	LabCore::ViewDescription createPlotViewDescription();
}