#pragma once
#include <LabCore/LabCoreAPI.h>

namespace PH::LabCore {

	bool32 pythonShellUpdate(void *instancedata);
	bool32 pythonShellInstantiate(void* instancedata);
	bool32 pythonShellDraw(void* instancedata);
	bool32 pythonShellDestroy(void* instancedata);
	bool32 pythonShellOnEvent(void* instancedata, const PH::Platform::Event& event);

	View getPythonShellView();
}