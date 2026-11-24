#include <LabCore/LabCoreAPI.h>
#include "views/PlotView.h"
#include "components/Grid.h"

namespace PH::LabCorePLT {




}


using namespace PH::LabCore;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {
		addView(PH::LabCorePLT::createPlotViewDescription());
		

		auto* lib = LabCore::getAssetLibrary();
		LabCore::Scene::addComponentDescription(LabCorePLT::Grid2D::createDescription());
	}
}