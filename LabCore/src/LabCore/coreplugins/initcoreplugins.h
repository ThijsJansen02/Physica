#pragma once
#include <LabCore/LabCoreAPI.h>
#include "PythonShell.h"

namespace PH::LabCore {
	void initCorePlugins() {	
		addView(getPythonShellView());
	}
}