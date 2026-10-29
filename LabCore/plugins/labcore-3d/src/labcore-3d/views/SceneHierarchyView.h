#pragma once

#include "../assets/Scene.h"
#include <LabCore/LabCoreAPI.h>
#include "../assets/GraphicsPipeline.h"

namespace PH::LabCore3D {

	struct SceneHierarchyView {
		Engine::AssetIdentifier* scene;
	};

	LabCore::ViewDescription createSceneHierarchyViewDescription();
}