#include <LabCore/LabCoreAPI.h>
#include "views/PlotView.h"


namespace PH::LabCorePLT {




}


using namespace PH::LabCore;

extern "C" {
	_declspec(dllexport) void init(const char* plugindir) {
		addView(PH::LabCorePLT::createPlotViewDescription());
	}
}