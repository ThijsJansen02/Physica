#include "SceneView.h"
#include <LabCore/LabCoreAPI.h>

namespace PH::LabCore3D {

	PH::bool32 drawSceneView(SceneView* data) {
		
		return true;
	}

	PH::bool32 updateSceneView(SceneView* data) {
		return true;
	}

	PH::bool32 instantiateSceneView(SceneView* data) {
		return true;
	}
	
	PH::bool32 destroySceneView(SceneView* data) {
		return true;
	}

	PH::bool32 SceneViewOnEvent(SceneView* data, const PH::Platform::Event& e) {
		return true;
	}

	PH::bool32 deserializeSceneView(SceneView* data, const YAML::Node& node) {
		return true;
	}

	PH::bool32 serializeSceneView(SceneView* data, YAML::Emitter& out) {
		return true;
	}

	LabCore::ViewDescription createSceneViewDescription() {
		LabCore::ViewDescription d{};
		d.draw_ = (LabCore::ViewDrawFunction)drawSceneView;
		d.onUpdate_ = (LabCore::ViewOnUpdateFunction)updateSceneView;
		d.onEvent_ = (LabCore::ViewOnEventFunction)SceneViewOnEvent;

		d.destroy_ = (LabCore::ViewDestroyFunction)destroySceneView;
		d.instantiate_ = (LabCore::ViewInstantiateFunction)instantiateSceneView;

		d.windowpadding = { 0,0 };
		d.windowflags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

		d.deserialize_ = (LabCore::ViewDeserializeFunction)deserializeSceneView;
		d.serialize_ = (LabCore::ViewSerializeFunction)serializeSceneView;

		d.name = Engine::String::create("Scene View");
		d.instancedatasize = sizeof(SceneView);

		return d;
	}
}