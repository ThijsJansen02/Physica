#include <LabCore/LabCoreAPI.h>
#include <Platform/platformAPI.h>
#include <imgui.h>

#include "SceneView.h"


extern "C" {
	_declspec(dllexport) void init() {

		PH::LabCore::addView(PH::LabCore3D::createSceneViewDescription());
	}
}