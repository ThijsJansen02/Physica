
#include "PythonShell.h"

namespace PH::LabCore {

	struct pythonShellInstance {
		Engine::String inputbuffer;
	};

	bool32 pythonShellUpdate(void* instancedata) {
		return true;
	}

	bool32 pythonShellInstantiate(void* instancedata) {
		return true;
	}

	bool32 pythonShellDraw(void* instancedata) {
		ImGui::Text("Python Shell");
		return true;
	}

	bool32 pythonShellDestroy(void* instancedata) {
		return true;
	}

	bool32 pythonShellOnEvent(void* instancedata, const PH::Platform::Event& event) {
		return true;
	}

	View getPythonShellView() {

		View v{};
		v.instancedatasize = sizeof(pythonShellInstance);
		v.destroy_ = pythonShellDestroy;
		v.draw_ = pythonShellDraw;
		v.onEvent_ = pythonShellOnEvent;
		v.onUpdate_ = pythonShellUpdate;
		v.instantiate_ = pythonShellInstantiate;
		v.name = Engine::String::create("Python Shell");

		return v;
	}
}